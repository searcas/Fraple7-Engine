#include "pch.h"
#include "AllocationFactory.h"
#include "DescriptorAllocator.h"
#include "DescriptorAllocation.h"
namespace Fraple7
{
	namespace Core
	{
		AllocationFactory& AllocationFactory::GetInstance()
		{
			static AllocationFactory factory;
			return factory;
		}

		DescriptorAllocation AllocationFactory::AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
		{
			return m_DescriptorAllocators[type]->Allocate(numDescriptors);
		}

		void AllocationFactory::ReleaseStaleDescriptors(uint64_t finishedFrame)
		{
			for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
			{
				m_DescriptorAllocators[i]->ReleaseStaleDescriptors(finishedFrame);
			}
		}

	}
}