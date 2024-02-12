#pragma once
#include "Utilities/Common/Common.h"
#include "directx/d3d12.h"
#include "Utilities/Thread/ThreadSafeQueue.hpp"

namespace Fraple7
{
	namespace Core
	{
		class CommandList;
		class CommandQueue
		{
		public:
			CommandQueue() = default;
			~CommandQueue();
			CommandQueue(D3D12_COMMAND_LIST_TYPE type);
			const ComPtr<ID3D12CommandQueue>& GetCmdQueue() const { return m_CommandQueue; }
		public:
			ComPtr<ID3D12GraphicsCommandList2> GetCommandList();
			std::shared_ptr<CommandList>GetCommandListClass();

			uint64_t ExecuteCommandList(const ComPtr<ID3D12GraphicsCommandList2>&);
			void WaitForFenceCompletion(uint64_t value);
			uint64_t SignalAndWait();

		public:
			void Join(const ComPtr<ID3D12GraphicsCommandList2>& commandList, const ComPtr<ID3D12Resource>& dst, const ComPtr<ID3D12Resource>& src);
			void Join(const ComPtr<ID3D12GraphicsCommandList2>& commandList, const ComPtr<ID3D12Resource>& dst, const ComPtr<ID3D12Resource>& src, size_t size, const std::vector<D3D12_SUBRESOURCE_DATA>& srcData);
			void Transition(const ComPtr<ID3D12Resource>& buffer, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
		private:
			// Keep track of command allocators that are "in-flight"
			struct CommandAllocatorBundle
			{
				uint64_t fenceValue;
				ComPtr<ID3D12CommandAllocator> commandAllocator;
			};
			void CreateCommandAllocatator();
			void CreateCommandList();

			bool IsFenceReached(uint64_t fenceVal);
			uint32_t CreateCommandQueue();
			void CreateAnEvent();
			void CreateFence();
			uint64_t Signal() const;

		private:
			ComPtr<ID3D12CommandQueue> m_CommandQueue;
			D3D12_COMMAND_QUEUE_DESC m_Desc;
			ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
			ComPtr<ID3D12GraphicsCommandList2> m_CommandList;
			std::queue<CommandAllocatorBundle> m_CommandAllocatorQueue;
			std::queue<ComPtr<ID3D12GraphicsCommandList2>> m_CommandListQueue;
			ComPtr<ID3D12Fence> m_Fence;
			mutable uint64_t m_FenceVal = 0;
			HANDLE m_FenceEvent;
			ThreadSafeQueue<std::shared_ptr<CommandList>> m_AvailableCommandLists;
			D3D12_COMMAND_LIST_TYPE m_CommandListType;
		};
	}
}