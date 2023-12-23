#pragma once
#include <initguid.h>
#include "Utilities/Common/Common.h"
#include <directx/d3dx12.h>
#include "DirectXTex/DirectXTex.h"
#include <string>

namespace Fraple7
{
	namespace Core
	{
		class Texture
		{
		public:
			Texture(const std::wstring& path, const ComPtr<ID3D12Device2>& device);
			~Texture();
			void Create();
			const D3D12_RESOURCE_DESC& GetTexDesc() { return m_TexDesc; }
			const std::vector<D3D12_SUBRESOURCE_DATA>& GetSubData() { return m_SubData; }
			const ComPtr<ID3D12Resource>& GetTextureBuffer() { return m_TextureBuffer; }
			const ComPtr<ID3D12Resource>& GetTextureUploadBuffer() { return m_TextureUploadBuffer; }
			const ComPtr<ID3D12DescriptorHeap>& GetSrvHeap() { return m_SrvHeap; }
			void DescriptorHeap();
			void ShaderResourceViewDesc();
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
			const ComPtr<ID3D12Device2>& m_Device;
		};
	}
}