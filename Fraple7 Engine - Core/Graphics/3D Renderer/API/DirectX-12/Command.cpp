#include "pch.h"
#include "Command.h"
#include "Utilities/Common/Common.h"

namespace Fraple7
{
	namespace Core
	{

		CommandMgr::CommandMgr(const ComPtr<ID3D12Device2>& device)
		{
			m_CommandQueueCopy = std::make_shared<Command::QueueDx>(device, D3D12_COMMAND_LIST_TYPE_COPY);
			m_CommandQueueDirect = std::make_shared<Command::QueueDx>(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
			m_CommandQueueCompute = std::make_shared<Command::QueueDx>(device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
		}
		
		CommandMgr::~CommandMgr()
		{
			UnloadAll();
		}
		const std::shared_ptr<Command::QueueDx>& CommandMgr::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
		{
			switch (type)
			{
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
				return m_CommandQueueDirect;
				break;
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
				return m_CommandQueueCompute;
				break;
			case D3D12_COMMAND_LIST_TYPE_COPY:
				return m_CommandQueueCopy;
				break;
			default:
				throw std::runtime_error{ "Failed Command Queue Type" };
				break;
			}
		}

		void CommandMgr::UnloadAll()
		{
			m_CommandQueueCopy->SignalAndWait();
			m_CommandQueueDirect->SignalAndWait();
			m_CommandQueueCompute->SignalAndWait();
		}


		Command::QueueDx::~QueueDx()
		{
			m_CommandQueue->Signal(m_Fence.Get(), ++m_FenceVal) >> statusCode;
			m_Fence->SetEventOnCompletion(m_FenceVal, m_FenceEvent) >> statusCode;

			if (WaitForSingleObject(m_FenceEvent, 2000) == WAIT_FAILED)
			{
				GetLastError() >> statusCode;
			}
		}
		Command::QueueDx::QueueDx(const ComPtr<ID3D12Device2>& device, D3D12_COMMAND_LIST_TYPE type) : m_Device(device) , m_Type(type)
		{
			const D3D12_COMMAND_QUEUE_DESC desc = {
				.Type = type,
				.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
				.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
				.NodeMask = 0
			};
			m_Desc = desc;
			CreateCommandQueue();
			CreateFence();
			CreateAnEvent();
		}
		ComPtr<ID3D12GraphicsCommandList2> Command::QueueDx::GetCommandList()
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
		void Command::QueueDx::CreateAnEvent()
		{
			m_FenceEvent = CreateEventW(nullptr, FALSE, FALSE, FALSE);
			if (!m_FenceEvent)
			{
				GetLastError() >> statusCode;
				throw std::runtime_error{ "Failed to create fence event" };
			}
		}
		void Command::QueueDx::CreateFence()
		{
			m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)) >> statusCode;
		}
		void Command::QueueDx::WaitForFenceCompletion(uint64_t fenceValue)
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
		uint64_t Command::QueueDx::Signal() const
		{
			m_CommandQueue->Signal(m_Fence.Get(), ++m_FenceVal) >> statusCode;
			return m_FenceVal;
		}
		uint64_t Command::QueueDx::SignalAndWait()
		{
			uint64_t signal = Signal();
			WaitForFenceCompletion(signal);
			return signal;
		}
		uint64_t Command::QueueDx::ExecuteCommandList(const ComPtr<ID3D12GraphicsCommandList2>& commandList)
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
		uint32_t Command::QueueDx::CreateCommandQueue()
		{
			m_Device->CreateCommandQueue(&m_Desc, IID_PPV_ARGS(&m_CommandQueue)) >> statusCode;
			return FPL_SUCCESS;
		}

		bool Command::QueueDx::IsFenceReached(uint64_t fenceVal)
		{
			return m_Fence->GetCompletedValue() >= fenceVal;
		}

		void Command::QueueDx::CreateCommandAllocatator()
		{
			m_Device->CreateCommandAllocator(m_Type, IID_PPV_ARGS(&m_CommandAllocator)) >> statusCode;
		}
		void Command::QueueDx::CreateCommandList()
		{
			m_Device->CreateCommandList(0, m_Type,
				m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)) >> statusCode;
		}
		void Command::QueueDx::Join(const ComPtr<ID3D12GraphicsCommandList2>& commandList, const ComPtr<ID3D12Resource>& dst, const ComPtr<ID3D12Resource>& src)
		{
			commandList->CopyResource(dst.Get(), src.Get());
		}
		void Command::QueueDx::Join(const ComPtr<ID3D12GraphicsCommandList2>& commandList, const ComPtr<ID3D12Resource>& dst, const ComPtr<ID3D12Resource>& src, size_t size, const std::vector<D3D12_SUBRESOURCE_DATA>& srcData)
		{	
			UpdateSubresources(commandList.Get(), dst.Get(), src.Get(), 0, 0, (UINT)size, srcData.data());
		}
		void Command::QueueDx::Transition(const ComPtr<ID3D12Resource>& buffer, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
		{
			const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), from, to);
			m_CommandList->ResourceBarrier(1, &barrier);
		}

}
}