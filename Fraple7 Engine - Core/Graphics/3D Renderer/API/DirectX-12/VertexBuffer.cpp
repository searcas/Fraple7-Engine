#include "pch.h"
#include "VertexBuffer.h"

namespace Fraple7
{
	namespace Core
	{
		VertexBuffer::VertexBuffer(const ComPtr<ID3D12Device2>& device)
		{
			ResourceMgr mgr;
			mgr.Allocate(device, m_VertexBuffer, m_Vertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COPY_DEST);
			mgr.Allocate(device, m_VertexUploadBuffer, m_Vertices, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

			Vertex* mappedVertexData = nullptr;
			m_VertexUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertexData)) >> statusCode;
			std::ranges::copy(s_VertexData, mappedVertexData);
			
		}

		VertexBuffer::~VertexBuffer()
		{
			m_VertexUploadBuffer->Unmap(0, nullptr);
		}
		void VertexBuffer::CreateVertexBufferView(const Vertex* vertex, uint32_t vertices)
		{
			const D3D12_VERTEX_BUFFER_VIEW vertexBufferView{
				.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress(),
				.SizeInBytes = vertices * sizeof(Vertex),
				.StrideInBytes = sizeof(Vertex)
			};
		}
	}
}

