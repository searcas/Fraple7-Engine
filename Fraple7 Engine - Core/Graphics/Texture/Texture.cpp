#include "pch.h"
#include "Texture.h"
#include "Graphics/3D Renderer/API/DirectX-12/ResourceMgr.h"
#include "Graphics/3D Renderer/API/DirectX-12/ResourceStateTracker.h"
#include "Graphics/3D Renderer/API/DirectX-12/Memory/AllocationFactory.h"
#include "Utilities/Utilities.hpp"
#include "Graphics/3D Renderer/API/DirectX-12/Device.h"
namespace Fraple7
{
	namespace Core
	{
		Texture::Texture(const std::wstring& path)
			: Resource(path)
		{
			CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY) >> statusCode;
			DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, m_Image) >> statusCode;
			DirectX::GenerateMipMaps(*m_Image.GetImages(), DirectX::TEX_FILTER_BOX, 0, m_mipChain) >> statusCode;
		}

		Texture::Texture(TextureUsage textureUsage, const std::wstring& name)
			:	Resource(name),
				m_TextureUsage(textureUsage)
		{

		}

		Texture::Texture(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue, TextureUsage textureUsage, const std::wstring& name)
			:	Resource(resourceDesc,clearValue, name),
				m_TextureUsage(textureUsage)
		{
			CreateViews();
		}

		Texture::Texture(ComPtr<ID3D12Resource> resource, TextureUsage textureUsage, const std::wstring& name)
			:	Resource(resource, name),
				m_TextureUsage(textureUsage)
		{
			CreateViews();
		}

		Texture::Texture(const Texture& copy)
			: Resource(copy)
		{
			CreateViews();
		}

		Texture::Texture(Texture&& move)
			: Resource(move)
		{
			CreateViews();
		}

		Texture& Texture::operator=(const Texture& copy)
		{
			Resource::operator=(copy);
			CreateViews();
			return *this;
		}

		Texture& Texture::operator=(Texture&& move)
		{
			Resource::operator=(move);
			CreateViews();
			return *this;
		}

		Texture::~Texture()
		{
		}
		void Texture::Resize(uint32_t width, uint32_t height, uint32_t depthOrArraySize)
		{

			if (m_d3d12Resource)
			{
				ResourceStateTracker::RemoveGlobalResourceState(m_d3d12Resource.Get());

				CD3DX12_RESOURCE_DESC resDesc(m_d3d12Resource->GetDesc());

				resDesc.Width = std::max(width, 1u);
				resDesc.Height = std::max(height, 1u);
				resDesc.DepthOrArraySize = depthOrArraySize;
				auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
				Device::GetDevice()->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE,
					&resDesc,
					D3D12_RESOURCE_STATE_COMMON,
					m_d3d12ClearValue.get(),
					IID_PPV_ARGS(&m_d3d12Resource)) >> statusCode;

				m_d3d12Resource->SetName(m_ResourceName.c_str());

				ResourceStateTracker::AddGlobalResourceState(m_d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON);

				CreateViews();
			}
		}
		void Texture::CreateViews()
		{

			if (m_d3d12Resource)
			{
				CD3DX12_RESOURCE_DESC desc(m_d3d12Resource->GetDesc());

				if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0 &&
					CheckRTVSupport())
				{
					m_RenderTargetView = AllocationFactory::GetInstance().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
					Device::GetDevice()->CreateRenderTargetView(m_d3d12Resource.Get(), nullptr, m_RenderTargetView.GetDescriptorHandle());
				}
				if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0 &&
					CheckDSVSupport())
				{
					m_DepthStencilView = AllocationFactory::GetInstance().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
					Device::GetDevice()->CreateDepthStencilView(m_d3d12Resource.Get(), nullptr, m_DepthStencilView.GetDescriptorHandle());
				}
			}

			std::lock_guard<std::mutex> lock(m_ShaderResourceViewsMutex);
			std::lock_guard<std::mutex> guard(m_UnorderedAccessViewsMutex);

			m_ShaderResourceViews.clear();
			m_UnorderedAccessViews.clear();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
		{
			std::size_t hash = 0;
			if (srvDesc)
			{
				hash = std::hash<D3D12_SHADER_RESOURCE_VIEW_DESC>{}(*srvDesc);
			}

			std::lock_guard<std::mutex> lock(m_ShaderResourceViewsMutex);

			auto iter = m_ShaderResourceViews.find(hash);
			if (iter == m_ShaderResourceViews.end())
			{
				auto srv = CreateShaderResourceView(srvDesc);
				iter = m_ShaderResourceViews.insert({ hash, std::move(srv) }).first;
			}
			return iter->second.GetDescriptorHandle();
		}

		// TODO: MAKE HASHER FOR UAV
		D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
		{
			std::size_t hash = 0;
			if (uavDesc)
			{
				hash = std::hash<D3D12_UNORDERED_ACCESS_VIEW_DESC>{}(*uavDesc);
			}

			std::lock_guard<std::mutex> lock(m_UnorderedAccessViewsMutex);

			auto iter = m_UnorderedAccessViews.find(hash);
			if (iter == m_UnorderedAccessViews.end())
			{
				auto uav = CreateUnorderedAccessView(uavDesc);
				iter = m_UnorderedAccessViews.insert({ hash, std::move(uav) }).first;
			}
			return iter->second.GetDescriptorHandle();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetRenderTargetView() const
		{
			return m_RenderTargetView.GetDescriptorHandle();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetDepthStencilView() const
		{
			return m_DepthStencilView.GetDescriptorHandle();
		}
		bool Texture::IsUAVCompatibleFormat(DXGI_FORMAT format)
		{
			switch (format)
			{
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SINT:
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SINT:
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT:
			case DXGI_FORMAT_R16_FLOAT:
			case DXGI_FORMAT_R16_UINT:
			case DXGI_FORMAT_R16_SINT:
			case DXGI_FORMAT_R8_UNORM:
			case DXGI_FORMAT_R8_UINT:
			case DXGI_FORMAT_R8_SINT:
				return true;
			default:
				return false;
			}
			return false;
		}
		bool Texture::IsSRGBFormat(DXGI_FORMAT format)
		{
			switch (format)
			{
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				return true;
			default:
				return false;
				break;
			}
			return false;
		}
		bool Texture::IsBGRFormat(DXGI_FORMAT format)
		{
			switch (format)
			{
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
				return true;
			default:
				return false;
				break;
			}
			return false;
		}
		bool Texture::IsDepthFormat(DXGI_FORMAT format)
		{
			switch (format)
			{
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
			case DXGI_FORMAT_D16_UNORM:
				return true;
			default:
				return false;
			}
			return false;
		}
		DXGI_FORMAT Texture::GetTypelessFormat(DXGI_FORMAT format)
		{
			DXGI_FORMAT typelessFormat = format;

			switch (format)
			{
			
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT:
				typelessFormat = DXGI_FORMAT_R32G32B32A32_TYPELESS;
				break;
			case DXGI_FORMAT_R32G32B32_FLOAT:
			case DXGI_FORMAT_R32G32B32_UINT:
			case DXGI_FORMAT_R32G32B32_SINT:
				typelessFormat = DXGI_FORMAT_R32G32B32_TYPELESS;
				break;
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UNORM:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SNORM:
			case DXGI_FORMAT_R16G16B16A16_SINT:
				typelessFormat = DXGI_FORMAT_R16G16B16A16_TYPELESS;
				break;
			case DXGI_FORMAT_R32G32_FLOAT:
			case DXGI_FORMAT_R32G32_UINT:
			case DXGI_FORMAT_R32G32_SINT:
				typelessFormat = DXGI_FORMAT_R32G32_TYPELESS;
				break;

			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
				typelessFormat = DXGI_FORMAT_R32G8X24_TYPELESS;
				break;

			case DXGI_FORMAT_R10G10B10A2_UNORM:
			case DXGI_FORMAT_R10G10B10A2_UINT:
				typelessFormat = DXGI_FORMAT_R10G10B10A2_TYPELESS;
				break;
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SNORM:
			case DXGI_FORMAT_R8G8B8A8_SINT:
				typelessFormat = DXGI_FORMAT_R8G8B8A8_TYPELESS;
				break;
			case DXGI_FORMAT_R16G16_FLOAT:
			case DXGI_FORMAT_R16G16_UNORM:
			case DXGI_FORMAT_R16G16_UINT:
			case DXGI_FORMAT_R16G16_SNORM:
			case DXGI_FORMAT_R16G16_SINT:
				typelessFormat = DXGI_FORMAT_R16G16_TYPELESS;
				break;
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT:
				typelessFormat = DXGI_FORMAT_R32_TYPELESS;
				break;
			case DXGI_FORMAT_R8G8_UNORM:
			case DXGI_FORMAT_R8G8_UINT:
			case DXGI_FORMAT_R8G8_SNORM:
			case DXGI_FORMAT_R8G8_SINT:
				typelessFormat = DXGI_FORMAT_R8G8_TYPELESS;
				break;
			case DXGI_FORMAT_R16_FLOAT:
			case DXGI_FORMAT_D16_UNORM:
			case DXGI_FORMAT_R16_UNORM:
			case DXGI_FORMAT_R16_UINT:
			case DXGI_FORMAT_R16_SNORM:
			case DXGI_FORMAT_R16_SINT:
				typelessFormat = DXGI_FORMAT_R16_TYPELESS;
				break;
			case DXGI_FORMAT_R8_UNORM:
			case DXGI_FORMAT_R8_UINT:
			case DXGI_FORMAT_R8_SNORM:
			case DXGI_FORMAT_R8_SINT:
				typelessFormat = DXGI_FORMAT_R8_TYPELESS;
				break;
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
				typelessFormat = DXGI_FORMAT_BC1_TYPELESS;
				break;
			case DXGI_FORMAT_BC2_TYPELESS:
			case DXGI_FORMAT_BC2_UNORM:
				typelessFormat = DXGI_FORMAT_BC2_TYPELESS;
				break;
			case DXGI_FORMAT_BC3_TYPELESS:
			case DXGI_FORMAT_BC3_UNORM:
				typelessFormat = DXGI_FORMAT_BC3_TYPELESS;
				break;
			case DXGI_FORMAT_BC4_TYPELESS:
			case DXGI_FORMAT_BC4_UNORM:
				typelessFormat = DXGI_FORMAT_BC4_TYPELESS;
				break;
			case DXGI_FORMAT_BC5_TYPELESS:
			case DXGI_FORMAT_BC5_UNORM:
				typelessFormat = DXGI_FORMAT_BC5_TYPELESS;
				break;
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
				typelessFormat = DXGI_FORMAT_B8G8R8A8_TYPELESS;
				break;
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
				typelessFormat = DXGI_FORMAT_B8G8R8X8_TYPELESS;
				break;
			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
				typelessFormat = DXGI_FORMAT_BC6H_TYPELESS;
				break;
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				typelessFormat = DXGI_FORMAT_BC7_TYPELESS;
				break;
			}
			return typelessFormat;
		}
		DXGI_FORMAT Texture::GetUAVCompatableFormat(DXGI_FORMAT format)
		{
			DXGI_FORMAT uavFormat = format;
			switch (format)
			{
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
				uavFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
				break;
			case DXGI_FORMAT_R32_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT:
				uavFormat = DXGI_FORMAT_R32_FLOAT;
				break;
			}
			return uavFormat;
		}
		void Texture::Create()
		{
			const auto& chainBase = *m_mipChain.GetImages();
			m_TexDesc = D3D12_RESOURCE_DESC{
				.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
				.Width = (UINT)chainBase.width,
				.Height = (UINT)chainBase.height,
				.DepthOrArraySize = 1,
				.MipLevels = (UINT16)m_mipChain.GetImageCount(),
				.Format = chainBase.format,
				.SampleDesc = {.Count = 1},
				.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
				.Flags = D3D12_RESOURCE_FLAG_NONE,
			};

			ResourceMgr::Allocate(m_TextureBuffer, m_TexDesc);
			// collect subresource data
			const auto subResourceData = std::ranges::views::iota(0, (int)m_mipChain.GetImageCount()) | std::ranges::views::transform([&](int i)
			{
					const auto img = m_mipChain.GetImage(i, 0, 0);

					return D3D12_SUBRESOURCE_DATA{
						.pData = img->pixels,
						.RowPitch = (LONG_PTR)img->rowPitch,
						.SlicePitch = (LONG_PTR)img->slicePitch,
					};
			}) | std::ranges::to<std::vector>();

			const auto uploadBufferSize = GetRequiredIntermediateSize(m_TextureBuffer.Get(), 0, (UINT)subResourceData.size());
			ResourceMgr::Allocate(m_TextureUploadBuffer, uploadBufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

			m_SubData.resize(m_mipChain.GetImageCount());
			memcpy(m_SubData.data(), subResourceData.data(), m_mipChain.GetImageCount() * sizeof(D3D12_SUBRESOURCE_DATA));
		}
		void Texture::DescriptorHeap()
		{
			const D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc
			{
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				.NumDescriptors = 1,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			};
			Device::GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvHeap)) >> statusCode;
			m_SrvHandle = m_SrvHeap->GetCPUDescriptorHandleForHeapStart();
		}
		void Texture::ShaderResourceViewDesc()
		{
			const D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{
				.Format = m_TextureBuffer->GetDesc().Format,
				.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D,
				.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
				.Texture2D{.MipLevels = m_TextureBuffer->GetDesc().MipLevels },
			};
			Device::GetDevice()->CreateShaderResourceView(m_TextureBuffer.Get(), &srvDesc, m_SrvHandle);
		}

		DescriptorAllocation Texture::CreateShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
		{
			return DescriptorAllocation();
		}

		DescriptorAllocation Texture::CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
		{
			return DescriptorAllocation();
		}
	
	}
}