#include "pch.h"
#include "IndexBuffer.h"
#include "ResourceMgr.h"
namespace Fraple7
{
	namespace Core
	{
		IndexBuffer::IndexBuffer()
		{
			
		}
		void IndexBuffer::Create(const ComPtr<ID3D12Device2>& device)
		{
			ResourceMgr::Allocate(device, m_IndexBuffer, sizeof(s_IndexData), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
			ResourceMgr::Allocate(device, m_IndexUploadBuffer, sizeof(s_IndexData), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

			WORD* mappedIndexData = nullptr;
			m_IndexUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndexData)) >> statusCode;
			std::ranges::copy(s_IndexData, mappedIndexData);
		}
		IndexBuffer::~IndexBuffer()
		{
			m_IndexUploadBuffer->Unmap(0, nullptr);
		}
		void IndexBuffer::CreateIndexBufferView(UINT vertices)
		{
			m_IndexBufferView = D3D12_INDEX_BUFFER_VIEW
			{
				.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress(),
				.SizeInBytes = (UINT)(m_Indices * sizeof(WORD)),
				.Format = DXGI_FORMAT_R16_UINT
			};
		}

	}
}
