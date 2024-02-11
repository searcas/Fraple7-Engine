#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3dx12.h"
namespace Fraple7
{
	namespace Core
	{
		class DescriptorAllocatorPage;
		class DescriptorAllocation
		{
		public:
			DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptorHandle, uint32_t numHandles, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> page);
			DescriptorAllocation();

			DescriptorAllocation(const DescriptorAllocation& copy) = delete;
			DescriptorAllocation& operator=(const DescriptorAllocation& copy) = delete;

			DescriptorAllocation(DescriptorAllocation&& other);
			DescriptorAllocation& operator=(DescriptorAllocation&& other);

			~DescriptorAllocation();
			bool IsNull();
			D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(uint32_t offset = 0)const;
			uint32_t GetNumHandles();

		private:
			std::shared_ptr<DescriptorAllocatorPage> GetDescriptorAllocatorPage()const;
			void Free();
			D3D12_CPU_DESCRIPTOR_HANDLE m_DescriptorHandle;
			uint32_t m_NumHandles;
			uint32_t m_DescriptorSize;

			std::shared_ptr<DescriptorAllocatorPage> m_Page;
		};

	}
}