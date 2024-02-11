#include "pch.h"
#include "UploadBuffer.h"
#include "Utilities/Management/Management.hpp"
#include "Graphics/3D Renderer/API/DirectX-12/Device.h"
namespace Fraple7
{
	namespace Core
	{
		using Allocation = UploadBuffer::Allocation;

		UploadBuffer::UploadBuffer(size_t pageSize) 
			: m_PageSize(pageSize)
		{

		}
		UploadBuffer::~UploadBuffer()
		{

		}
		Allocation UploadBuffer::Allocate(size_t sizeInBytes, size_t alignment)
		{
			if (sizeInBytes > m_PageSize)
			{
				throw std::bad_alloc();
			}

			if (!m_CurrentPage || !m_CurrentPage->HasSpace(sizeInBytes, alignment))
			{
				m_CurrentPage = RegustPage();
			}

			return m_CurrentPage->Allocate(sizeInBytes, alignment);
		}
		void UploadBuffer::Reset()
		{
			m_CurrentPage = nullptr;

			m_AvailablePages = m_PagePool;

			for (const auto& page : m_AvailablePages)
			{
				page->Reset();
			}
		}
		std::shared_ptr<UploadBuffer::Page> UploadBuffer::RegustPage()
		{
			std::shared_ptr<Page>page;

			if (!m_AvailablePages.empty())
			{
				page = m_AvailablePages.front();
				m_AvailablePages.pop_front();
			}
			else
			{
				page = std::make_shared<Page>(m_PageSize);
				m_PagePool.push_back(page);
			}
			return page;
		}
		UploadBuffer::Page::Page(size_t sizeInBytes)
			:	m_PageSize(sizeInBytes),
				m_Offset(0),
				m_CPUptr(nullptr),
				m_GPUptr(D3D12_GPU_VIRTUAL_ADDRESS(0))
			
		{
			CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
			const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_PageSize);

			Device::GetDevice()->CreateCommittedResource(&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_d3d12Resource)) >> statusCode;
			m_GPUptr = m_d3d12Resource->GetGPUVirtualAddress();
			m_d3d12Resource->Map(0, nullptr, &m_CPUptr);
		}
		UploadBuffer::Page::~Page()
		{
			m_d3d12Resource->Unmap(0, nullptr);
			m_CPUptr = nullptr;
			m_GPUptr = D3D12_GPU_VIRTUAL_ADDRESS(0);
		}
		bool UploadBuffer::Page::HasSpace(size_t sizeInBytes, size_t alignment) const
		{
			size_t alignedSize = ::AlignUp(sizeInBytes, alignment);
			size_t alignedOffset = ::AlignUp(m_Offset, alignment);

			return alignedOffset + alignedSize <= m_PageSize;
		}
		Allocation UploadBuffer::Page::Allocate(size_t sizeInBytes, size_t alignment)
		{
			if (!HasSpace(sizeInBytes, alignment))
			{
				throw std::bad_alloc();
			}

			size_t alignedSize = ::AlignUp(sizeInBytes, alignment);
			m_Offset = ::AlignUp(m_Offset, alignment);

			Allocation allocation = { };

			allocation.CPU = static_cast<uint8_t*>(m_CPUptr) + m_Offset;
			allocation.GPU = m_GPUptr + m_Offset;

			m_Offset += alignedSize;
			return allocation;
		}
		void UploadBuffer::Page::Reset()
		{
			m_Offset = 0;
		}
	}
}