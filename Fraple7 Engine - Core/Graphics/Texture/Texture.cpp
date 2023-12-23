#include "pch.h"
#include "Texture.h"
#include "Graphics/3D Renderer/API/DirectX-12/ResourceMgr.h"

namespace Fraple7
{
	namespace Core
	{
		Texture::Texture(const std::wstring& path, const ComPtr<ID3D12Device2>& device) : m_Device(device)
		{
			CoInitializeEx(nullptr, COINIT_SPEED_OVER_MEMORY) >> statusCode;
			DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, m_Image) >> statusCode;
			DirectX::GenerateMipMaps(*m_Image.GetImages(), DirectX::TEX_FILTER_BOX, 0, m_mipChain) >> statusCode;
		}

		Texture::~Texture()
		{
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

			ResourceMgr::Allocate(m_Device, m_TextureBuffer, m_TexDesc);
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
			ResourceMgr::Allocate(m_Device, m_TextureUploadBuffer, uploadBufferSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

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
			m_Device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_SrvHeap)) >> statusCode;
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
			m_Device->CreateShaderResourceView(m_TextureBuffer.Get(), &srvDesc, m_SrvHandle);
		}
	
	}
}