#pragma once
#include "directx/d3dx12.h"

namespace Fraple7
{
	namespace Core
	{
		class DescriptorAllocator;
		class DescriptorAllocation;
		class AllocationFactory
		{
		public:
			static AllocationFactory& GetInstance();
			AllocationFactory(const AllocationFactory& copy) = delete;
			AllocationFactory(AllocationFactory&&move) = delete;
			AllocationFactory& operator= (const AllocationFactory& copy) = delete;
			AllocationFactory& operator= (AllocationFactory&& copy) = delete;
			DescriptorAllocation AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors = 1);
			void ReleaseStaleDescriptors(uint64_t finishedFrame);
		private:
			AllocationFactory() = default;
			std::unique_ptr<DescriptorAllocator> m_DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		};
	}
}
