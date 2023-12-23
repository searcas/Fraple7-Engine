#pragma once
#include <DirectXMath.h>
#include <Utilities/Common/Common.h>
#include "directx/d3d12.h"
#include "ResourceMgr.h"
namespace Fraple7
{
	namespace Core
	{
		struct Vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT2 texCoord;
		};
		constexpr Vertex s_VertexData[] = {
			{ { -1.0f,   -1.0f, -1.0f}, {0.00f, 0.0f}, },
			{ { -1.0f,    1.0f, -1.0f}, {0.00f, 1.0f}, },
			{ {  1.0f,    1.0f, -1.0f}, {1.00f, 1.0f}, },
			{ {  1.0f,   -1.0f, -1.0f}, {1.00f, 0.0f}, },
			{ { -1.0f,   -1.0f,  1.0f}, {0.00f, 1.0f}, },
			{ { -1.0f,    1.0f,  1.0f}, {0.00f, 0.0f}, },
			{ {  1.0f,    1.0f,  1.0f}, {1.00f, 0.0f}, },
			{ {  1.0f,   -1.0f,  1.0f}, {1.00f, 1.0f}, },
		}; 
		class VertexBuffer
		{
		public:
			VertexBuffer();
			void Create(const ComPtr<ID3D12Device2>& device);
			~VertexBuffer();
			const ComPtr<ID3D12Resource>& GetVertexBuffer() const { return m_VertexBuffer; }
			const ComPtr<ID3D12Resource>& GetVertexUploadBuffer() const { return m_VertexUploadBuffer; }
			void CreateVertexBufferView(UINT vertices = std::size(s_VertexData));
			const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() { return m_VertexBufferView; }
			uint32_t GetNumVertices() { return m_Vertices; }
		private:
			ComPtr<ID3D12Resource> m_VertexBuffer;
			ComPtr<ID3D12Resource> m_VertexUploadBuffer;
			uint32_t m_Vertices = std::size(s_VertexData);
			D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
		};
	}
}