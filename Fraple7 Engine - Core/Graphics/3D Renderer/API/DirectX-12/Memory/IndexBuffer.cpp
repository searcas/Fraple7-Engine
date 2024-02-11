#include "pch.h"
#include "IndexBuffer.h"
#include "Graphics/3D Renderer/API/DirectX-12/ResourceMgr.h"
namespace Fraple7
{
	namespace Core
	{
		IndexBuffer::IndexBuffer(const std::wstring& name) 
			:	Buffer(name),
				m_IndexBufferView({})
		{
			
		}
		void IndexBuffer::Create()
		{
			ResourceMgr::Allocate(m_IndexBuffer, sizeof(s_IndexData), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON);
			ResourceMgr::Allocate(m_IndexUploadBuffer, sizeof(s_IndexData), D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

			WORD* mappedIndexData = nullptr;
			m_IndexUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndexData)) >> statusCode;
			std::ranges::copy(s_IndexData, mappedIndexData);
		}
		D3D12_CPU_DESCRIPTOR_HANDLE IndexBuffer::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
		{
			return D3D12_CPU_DESCRIPTOR_HANDLE();
		}
		D3D12_CPU_DESCRIPTOR_HANDLE IndexBuffer::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
		{
			return D3D12_CPU_DESCRIPTOR_HANDLE();
		}
		void IndexBuffer::CreateViews(size_t numeElements, size_t elementSize)
		{
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
