#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3d12.h"


namespace Fraple7
{
	namespace Core
	{
		class DescriptorAllocation;
		class DescriptorAllocatorPage;

		class DescriptorAllocator
		{
		public:
			DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap);
			~DescriptorAllocator();

			DescriptorAllocation Allocate(uint32_t numDescriptors = 1);

			void ReleaseStaleDescriptors(uint64_t fameNumber);
			  
		private:
			using DescriptorHeapPool = std::vector<std::shared_ptr<DescriptorAllocatorPage>>;
			std::shared_ptr<DescriptorAllocatorPage> CreateAllocatorPage();
			D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
			uint32_t m_NumDescriptorsPerHeap = 256;
			DescriptorHeapPool m_HeapPool;
			std::set<size_t> m_AvailableHeaps;
			std::mutex m_AllocationMutex;
		};
	}
}