#include "pch.h"
#include "CommandQueue.h"
#include "CommandList.h"
#include "Device.h"

namespace Fraple7
{
	namespace Core
	{
		CommandQueue::~CommandQueue()
		{
			m_CommandQueue->Signal(m_Fence.Get(), ++m_FenceVal) >> statusCode;
			m_Fence->SetEventOnCompletion(m_FenceVal, m_FenceEvent) >> statusCode;

			if (WaitForSingleObject(m_FenceEvent, 2000) == WAIT_FAILED)
			{
				GetLastError() >> statusCode;
			}
		}
		CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE commandListType) :
			m_CommandListType(commandListType)
		{
			const D3D12_COMMAND_QUEUE_DESC desc = {
				.Type = commandListType,
				.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
				.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
				.NodeMask = 0
			};
			m_Desc = desc;
			CreateCommandQueue();
			CreateFence();
			CreateAnEvent();
		}
		ComPtr<ID3D12GraphicsCommandList2> CommandQueue::GetCommandList()
		{
			if (!m_CommandAllocatorQueue.empty() && IsFenceReached(m_CommandAllocatorQueue.front().fenceValue))
			{
				m_CommandAllocator = m_CommandAllocatorQueue.front().commandAllocator;
				m_CommandAllocatorQueue.pop();
				m_CommandAllocator->Reset() >> statusCode;
			}
			else
			{
				CreateCommandAllocatator();
			}
			if (!m_CommandListQueue.empty())
			{
				m_CommandList = m_CommandListQueue.front();
				m_CommandListQueue.pop();
				m_CommandList->Reset(m_CommandAllocator.Get(), nullptr) >> statusCode;
			}
			else
			{
				CreateCommandList();
			}
			// Bind command allocator with command list 
			// so we can get when command list is executed
			m_CommandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), m_CommandAllocator.Get()) >> statusCode;
			return m_CommandList;
		}


		std::shared_ptr<CommandList> CommandQueue::GetCommandListClass()
		{
			std::shared_ptr<CommandList> commandList;
			if (!m_AvailableCommandLists.Empty())
			{
				m_AvailableCommandLists.TryPop(commandList);
			}
			else
			{
				commandList = std::make_shared<CommandList>(m_CommandListType);
			}
			return commandList;
		}
		void CommandQueue::CreateAnEvent()
		{
			m_FenceEvent = CreateEventW(nullptr, FALSE, FALSE, FALSE);
			if (!m_FenceEvent)
			{
				GetLastError() >> statusCode;
				throw std::runtime_error{ "Failed to create fence event" };
			}
		}
		void CommandQueue::CreateFence()
		{
			Device::GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)) >> statusCode;
		}
		void CommandQueue::WaitForFenceCompletion(uint64_t fenceValue)
		{
			if (!IsFenceReached(fenceValue))
			{
				m_Fence->SetEventOnCompletion(m_FenceVal, m_FenceEvent) >> statusCode;
				if (::WaitForSingleObject(m_FenceEvent, DWORD_MAX) == WAIT_FAILED)
				{
					GetLastError() >> statusCode;
				}
			}
		}
		uint64_t CommandQueue::Signal() const
		{
			m_CommandQueue->Signal(m_Fence.Get(), ++m_FenceVal) >> statusCode;
			return m_FenceVal;
		}
		uint64_t CommandQueue::SignalAndWait()
		{
			uint64_t signal = Signal();
			WaitForFenceCompletion(signal);
			return signal;
		}
		uint64_t CommandQueue::ExecuteCommandList(const ComPtr<ID3D12GraphicsCommandList2>& commandList)
		{
			commandList->Close();
			ID3D12CommandAllocator* commandAllocator;
			UINT dataSize = sizeof(commandAllocator);

			commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator) >> statusCode;

			ID3D12CommandList* const pComandLists[] = { commandList.Get() };
			m_CommandQueue->ExecuteCommandLists(1, pComandLists);

			uint64_t fenceValue = Signal();

			m_CommandAllocatorQueue.emplace(CommandAllocatorBundle{ fenceValue, commandAllocator });
			m_CommandListQueue.push(commandList);

			// We can safe release command allocator
			// Since we transfered in the command allocator queue.
			m_CommandAllocator->Release();
			return fenceValue;
		}
		uint32_t CommandQueue::CreateCommandQueue()
		{
			Device::GetDevice()->CreateCommandQueue(&m_Desc, IID_PPV_ARGS(&m_CommandQueue)) >> statusCode;
			return FPL_SUCCESS;
		}

		bool CommandQueue::IsFenceReached(uint64_t fenceVal)
		{
			return m_Fence->GetCompletedValue() >= fenceVal;
		}

		void CommandQueue::CreateCommandAllocatator()
		{
			Device::GetDevice()->CreateCommandAllocator(m_CommandListType, IID_PPV_ARGS(&m_CommandAllocator)) >> statusCode;
		}
		void CommandQueue::CreateCommandList()
		{
			Device::GetDevice()->CreateCommandList(0, m_CommandListType,
				m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)) >> statusCode;
		}
		void CommandQueue::Join(const ComPtr<ID3D12GraphicsCommandList2>& commandList, const ComPtr<ID3D12Resource>& dst, const ComPtr<ID3D12Resource>& src)
		{
			commandList->CopyResource(dst.Get(), src.Get());
		}
		void CommandQueue::Join(const ComPtr<ID3D12GraphicsCommandList2>& commandList, const ComPtr<ID3D12Resource>& dst, const ComPtr<ID3D12Resource>& src, size_t size, const std::vector<D3D12_SUBRESOURCE_DATA>& srcData)
		{
			UpdateSubresources(commandList.Get(), dst.Get(), src.Get(), 0, 0, (UINT)size, srcData.data());
		}
		void CommandQueue::Transition(const ComPtr<ID3D12Resource>& buffer, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
		{
			const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), from, to);
			m_CommandList->ResourceBarrier(1, &barrier);
		}
	}
}