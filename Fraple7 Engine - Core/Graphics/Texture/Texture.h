#pragma once
#include <initguid.h>
#include "Utilities/Common/Common.h"
#include <directx/d3dx12.h>
#include "DirectXTex/DirectXTex.h"
#include "Graphics/3D Renderer/API/DirectX-12/Resource.h"
#include "Graphics/3D Renderer/API/DirectX-12/Memory/DescriptorAllocation.h"
namespace Fraple7
{
	namespace Core
	{
		enum class TextureUsage
		{
			Albedo,
			Diffuse = Albedo,       // Treat Diffuse and Albedo textures the same.
			Heightmap,
			Depth = Heightmap,      // Treat height and depth textures the same.
			Normalmap,
			RenderTarget,           // Texture is used as a render target.
		};
		class Texture : public Resource
		{
		public:
			Texture(const std::wstring& path);

			explicit Texture(TextureUsage textureUsage = TextureUsage::Albedo,
							 const std::wstring& name = L"");

			explicit Texture(const D3D12_RESOURCE_DESC& resourceDesc,
							 const D3D12_CLEAR_VALUE* clearValue = nullptr,
							 TextureUsage textureUsage = TextureUsage::Albedo,
							 const std::wstring& name = L"");
			
			explicit Texture(ComPtr<ID3D12Resource> resource,
							 TextureUsage textureUsage = TextureUsage::Albedo,
							 const std::wstring& name = L"");

			Texture(const Texture& copy);
			Texture(Texture&& move);

			Texture& operator=(const Texture& copy);
			Texture& operator=(Texture&& move);

			virtual ~Texture();

			TextureUsage GetTextureUsage() const
			{
				return m_TextureUsage;
			}
			void SetTextureUsage(TextureUsage textureUsage)
			{
				m_TextureUsage = textureUsage;
			}

			void Resize(uint32_t width, uint32_t height, uint32_t depthOrArraySize = 1);

			virtual void CreateViews();

			virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const override;
			virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override;
			virtual D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const;
			virtual D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;

			bool CheckSRVSupport() const
			{
				return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE);
			}
			bool CheckRTVSupport() const
			{
				return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_RENDER_TARGET);
			}
			bool CheckUAVSupport()
			{
				return	CheckFormatSupport(D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) &&
						CheckFormatSupport(D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) &&
						CheckFormatSupport(D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE);
			}
			bool CheckDSVSupport() const
			{
				return CheckFormatSupport(D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL);
			}
			static bool IsUAVCompatibleFormat(DXGI_FORMAT format);
			static bool IsSRGBFormat(DXGI_FORMAT format);
			static bool IsBGRFormat(DXGI_FORMAT format);
			static bool IsDepthFormat(DXGI_FORMAT format);
			
			static DXGI_FORMAT GetTypelessFormat(DXGI_FORMAT format);
			static DXGI_FORMAT GetUAVCompatableFormat(DXGI_FORMAT format);



			void Create();
			const D3D12_RESOURCE_DESC& GetTexDesc() { return m_TexDesc; }
			const std::vector<D3D12_SUBRESOURCE_DATA>& GetSubData() { return m_SubData; }
			const ComPtr<ID3D12Resource>& GetTextureBuffer() { return m_TextureBuffer; }
			const ComPtr<ID3D12Resource>& GetTextureUploadBuffer() { return m_TextureUploadBuffer; }
			const ComPtr<ID3D12DescriptorHeap>& GetSrvHeap() { return m_SrvHeap; }
			void DescriptorHeap();
			void ShaderResourceViewDesc();
		private:
			DescriptorAllocation CreateShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const;
			DescriptorAllocation CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const;

			mutable std::unordered_map<size_t, DescriptorAllocation> m_ShaderResourceViews;
			mutable std::unordered_map<size_t, DescriptorAllocation> m_UnorderedAccessViews;

			mutable std::mutex m_ShaderResourceViewsMutex;
			mutable std::mutex m_UnorderedAccessViewsMutex;

			DescriptorAllocation m_RenderTargetView;
			DescriptorAllocation m_DepthStencilView;

			TextureUsage m_TextureUsage;

		private:
			ComPtr<ID3D12Resource> m_TextureBuffer;
			ComPtr<ID3D12Resource> m_TextureUploadBuffer;
			DirectX::ScratchImage m_Image;
			DirectX::ScratchImage m_mipChain;
			D3D12_RESOURCE_DESC m_TexDesc;
			std::vector<D3D12_SUBRESOURCE_DATA> m_SubData;
			// Shader Resource View
			ComPtr<ID3D12DescriptorHeap> m_SrvHeap;
			CD3DX12_CPU_DESCRIPTOR_HANDLE m_SrvHandle;
		};
	}
}