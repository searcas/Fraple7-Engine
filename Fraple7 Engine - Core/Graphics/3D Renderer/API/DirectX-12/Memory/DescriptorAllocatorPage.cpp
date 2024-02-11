#include "pch.h"
#include "DescriptorAllocatorPage.h"
#include "Graphics/3D Renderer/API/DirectX-12/Device.h"
namespace Fraple7
{
	namespace Core
	{
		DescriptorAllocatorPage::DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) 
			: m_HeapType(type), m_NumDescriptorsInHeap(numDescriptors)
			
		{
			D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
			heapDesc.Type = m_HeapType;
			heapDesc.NumDescriptors = m_NumDescriptorsInHeap;

			Device::GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_D3D12DescriptorHeap)) >> statusCode;

			m_BaseDescriptor = m_D3D12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			m_DescriptorHandleIncrementSize = Device::GetDevice()->GetDescriptorHandleIncrementSize(m_HeapType);
			m_NumFreeHandles = m_NumDescriptorsInHeap;

			// Initialize the free lists
			AddNewBlock(0, m_NumFreeHandles);
		}
		D3D12_DESCRIPTOR_HEAP_TYPE DescriptorAllocatorPage::GetHeapType() const
		{
			return m_HeapType;
		}
		DescriptorAllocatorPage::DescriptorAllocatorPage()
		{
		}
		bool DescriptorAllocatorPage::HasSpace(uint32_t numDescriptors) const
		{
			return m_FreeListBySize.lower_bound(numDescriptors) != m_FreeListBySize.end();
		}
		DescriptorAllocatorPage::~DescriptorAllocatorPage()
		{
		}
		uint32_t DescriptorAllocatorPage::NumFreeHandles() const
		{
			return m_NumFreeHandles;
		}
		DescriptorAllocation DescriptorAllocatorPage::Allocate(uint32_t numDescriptors)
		{
			std::lock_guard<std::mutex>lock(m_AllocationMutex);

			if (numDescriptors > m_NumFreeHandles)
			{
				return DescriptorAllocation();
			}
			auto smallestBlock_It = m_FreeListBySize.lower_bound(numDescriptors);
			if (smallestBlock_It == m_FreeListBySize.end())
			{

				return DescriptorAllocation();
			}
			auto blockSize = smallestBlock_It->first;
			auto offset_It = smallestBlock_It->second;

			auto offset = offset_It->first;

			m_FreeListBySize.erase(smallestBlock_It);
			m_FreeListByOffset.erase(offset_It);

			auto newOffset = offset + numDescriptors;
			auto newSize = blockSize - numDescriptors;

			if (newSize > 0)
			{
				AddNewBlock(newOffset, newSize);
			}
			m_NumFreeHandles -= numDescriptors;

			return DescriptorAllocation(
				CD3DX12_CPU_DESCRIPTOR_HANDLE(m_BaseDescriptor, offset, m_DescriptorHandleIncrementSize),
				numDescriptors, m_DescriptorHandleIncrementSize, shared_from_this());
		}
		void DescriptorAllocatorPage::Free(DescriptorAllocation&& descriptorHandle, uint64_t frameNumber)
		{
			auto offset = ComputeOffset(descriptorHandle.GetDescriptorHandle());

			std::lock_guard<std::mutex> lock(m_AllocationMutex);

			m_StaleDescriptors.emplace(offset, descriptorHandle.GetNumHandles(), frameNumber);
		}
		void DescriptorAllocatorPage::RleaseStaleDescriptors(uint64_t frameNumber)
		{
			std::lock_guard<std::mutex> lock(m_AllocationMutex);

			while (!m_StaleDescriptors.empty() && m_StaleDescriptors.front().FrameNumber <= frameNumber)
			{
				auto& staleDescriptor = m_StaleDescriptors.front();

				auto offset = staleDescriptor.Offset;

				auto numDescriptors = staleDescriptor.Size;

				FreeBlock(offset, numDescriptors);

				m_StaleDescriptors.pop();
			}
		}
		uint32_t DescriptorAllocatorPage::ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle)
		{
			return static_cast<uint32_t>(handle.ptr - m_BaseDescriptor.ptr) / m_DescriptorHandleIncrementSize;
		}
		void DescriptorAllocatorPage::AddNewBlock(uint32_t offset, uint32_t numDescriptors)
		{
			auto offset_it = m_FreeListByOffset.emplace(offset, numDescriptors);
			auto size_it = m_FreeListBySize.emplace(numDescriptors, offset_it.first);
			offset_it.first->second.FreeListBySize_It = size_it;
		}
		void DescriptorAllocatorPage::FreeBlock(uint32_t offset, uint32_t numDescriptors)
		{
			auto nextBlock_it = m_FreeListByOffset.upper_bound(offset);

			auto prevBlock_it = nextBlock_it;

			if (prevBlock_it != m_FreeListByOffset.begin())
			{
				--prevBlock_it;
			}
			else
			{
				prevBlock_it = m_FreeListByOffset.end();
			}

			m_NumFreeHandles += numDescriptors;

			if (prevBlock_it != m_FreeListByOffset.end() && offset == prevBlock_it->first + prevBlock_it->second.Size)
			{
				offset = prevBlock_it->first;
				numDescriptors += prevBlock_it->second.Size;

				m_FreeListBySize.erase(prevBlock_it->second.FreeListBySize_It);
				m_FreeListByOffset.erase(prevBlock_it);
			}
			if (nextBlock_it != m_FreeListByOffset.end() && offset + numDescriptors == nextBlock_it->first)
			{
				numDescriptors += nextBlock_it->second.Size;

				m_FreeListBySize.erase(nextBlock_it->second.FreeListBySize_It);
				m_FreeListByOffset.erase(nextBlock_it);
			}

			AddNewBlock(offset, numDescriptors);
		}
	}
}