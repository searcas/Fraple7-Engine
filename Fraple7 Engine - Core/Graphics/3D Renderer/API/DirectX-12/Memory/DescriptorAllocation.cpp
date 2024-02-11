#include "pch.h"
#include "DescriptorAllocation.h"
#include "DescriptorAllocatorPage.h"
namespace Fraple7
{
	namespace Core
	{
		DescriptorAllocation::DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, uint32_t numHandles, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> page)
			: m_DescriptorHandle{ descriptorHandle },
			  m_NumHandles(numHandles),
			  m_DescriptorSize(descriptorSize),
			  m_Page(page)
		{

		}
		DescriptorAllocation::DescriptorAllocation()
			: m_DescriptorHandle{ 0 },
			m_NumHandles(0),
			m_DescriptorSize(0),
			m_Page(nullptr)
		{
		}
		DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& other)
			: m_DescriptorHandle( other.m_DescriptorHandle ),
			m_NumHandles(other.m_NumHandles),
			m_DescriptorSize(other.m_DescriptorSize),
			m_Page(other.m_Page)
		{
			other.m_DescriptorHandle.ptr = 0;
			other.m_NumHandles = 0;
			other.m_DescriptorSize = 0;
		}
		DescriptorAllocation& DescriptorAllocation::operator=(DescriptorAllocation&& other)
		{
			Free();

			m_DescriptorHandle = other.m_DescriptorHandle;
			m_NumHandles = other.m_NumHandles;
			m_DescriptorSize = other.m_DescriptorSize;
			m_Page = std::move(other.m_Page);

			other.m_DescriptorHandle.ptr = 0;
			other.m_NumHandles = 0;
			other.m_DescriptorSize = 0;

			return *this;
		}
		DescriptorAllocation::~DescriptorAllocation()
		{
			Free();
		}
		bool DescriptorAllocation::IsNull()
		{
			return m_DescriptorHandle.ptr == 0;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::GetDescriptorHandle(uint32_t offset) const
		{
			assert(offset < m_NumHandles);
			return { m_DescriptorHandle.ptr + (m_DescriptorSize * offset) };
		}
		uint32_t DescriptorAllocation::GetNumHandles()
		{
			return m_NumHandles;
		}
		std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocation::GetDescriptorAllocatorPage() const
		{
			return m_Page;
		}
		void DescriptorAllocation::Free()
		{
			if (!IsNull() && m_Page)
			{
				m_Page->Free(std::move(*this), 3);
				m_DescriptorHandle.ptr = 0;
				m_NumHandles = 0;
				m_DescriptorSize = 0;
				m_Page.reset();
			}
		}
	}
}