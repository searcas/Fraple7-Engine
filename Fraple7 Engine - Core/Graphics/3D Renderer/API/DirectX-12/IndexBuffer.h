#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3dx12.h"
#include <DirectXMath.h>
namespace Fraple7
{
	namespace Core
	{
		const WORD s_IndexData[] = {
				0, 1, 2, 0, 2, 3,
				4, 6, 5, 4, 7, 6,
				4, 5, 1, 4, 1, 0,
				3, 2, 6, 3, 6, 7,
				1, 5, 6, 1, 6, 2,
				4, 0, 3, 4, 3, 7
		};
		const DirectX::XMFLOAT4 s_FaceColors[] = {
			{1.00f, 0.67f, 1.00f, 1.f},
			{1.00f, 0.01f, 0.30f, 1.f},
			{0.00f, 1.00f, 0.30f, 1.f},
			{0.69f, 0.68f, 0.30f, 1.f},
			{0.69f, 0.30f, 0.40f, 1.f},
			{0.69f, 0.33f, 0.90f, 1.f},
		};
		class IndexBuffer
		{
		
		public:
			IndexBuffer();
			~IndexBuffer();
			const ComPtr<ID3D12Resource>& GetIndexBuffer() { return m_IndexBuffer; }
			const ComPtr<ID3D12Resource>& GetIndexUploadBuffer() { return m_IndexUploadBuffer; }
			const ComPtr<ID3D12Resource>& GetFaceColorUploadBuffer() { return m_FaceColorUploadBuffer; }
			const ComPtr<ID3D12Resource>& GetFaceColorBuffer() { return m_FaceColorBuffer; }
			void CreateIndexBufferView(UINT vertices = std::size(s_IndexData));
			const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() { return m_IndexBufferView; }
			void Create(const ComPtr<ID3D12Device2>&);
			void CreateColorSide(const ComPtr<ID3D12Device2>&);
			UINT GetIndices() { return m_Indices; }
		private:
			ComPtr<ID3D12Resource> m_IndexBuffer;
			ComPtr<ID3D12Resource> m_IndexUploadBuffer;
			UINT m_Indices = std::size(s_IndexData);
			D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
			ComPtr<ID3D12Resource> m_FaceColorBuffer;
			ComPtr<ID3D12Resource> m_FaceColorUploadBuffer;

		};
	}
}
