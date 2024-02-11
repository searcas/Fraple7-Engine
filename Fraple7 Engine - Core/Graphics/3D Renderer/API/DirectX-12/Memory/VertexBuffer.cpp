#include "pch.h"
#include "VertexBuffer.h"

namespace Fraple7
{
	namespace Core
	{
		VertexBuffer::VertexBuffer(const std::wstring& name)
			: Buffer(name)
		{
		
		}

		void VertexBuffer::Create()
		{
			
			ResourceMgr::Allocate(m_VertexBuffer, sizeof(s_VertexData), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
			ResourceMgr::Allocate(m_VertexUploadBuffer, sizeof(s_VertexData), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

			Vertex* mappedVertexData = nullptr;
			m_VertexUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedVertexData)) >> statusCode;
			std::ranges::copy(s_VertexData, mappedVertexData);
		}
	
		VertexBuffer::~VertexBuffer()
		{
			m_VertexUploadBuffer->Unmap(0, nullptr);
		}
		void VertexBuffer::CreateVertexBufferView(UINT vertices)
		{
			m_VertexBufferView = D3D12_VERTEX_BUFFER_VIEW
			{
				.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress(),
				.SizeInBytes = (UINT)(vertices * sizeof(Vertex)),
				.StrideInBytes = sizeof(Vertex)
			};
		}
		D3D12_CPU_DESCRIPTOR_HANDLE VertexBuffer::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
		{
			return D3D12_CPU_DESCRIPTOR_HANDLE();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE VertexBuffer::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
		{
			return D3D12_CPU_DESCRIPTOR_HANDLE();
		}
		void VertexBuffer::CreateViews(size_t numeElements, size_t elementSize)
		{

		}
	}
}

