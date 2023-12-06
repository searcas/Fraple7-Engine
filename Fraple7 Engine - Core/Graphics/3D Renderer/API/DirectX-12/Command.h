#pragma once
#include <wrl.h>
#include <dxgi1_6.h>
#include "d3d12.h"

namespace Fraple7
{
	namespace Core
	{
		using Microsoft::WRL::ComPtr;

		class Commands
		{
		public:
			class Queue
			{
			public:
				Queue() = default;
				Queue(const D3D12_COMMAND_QUEUE_DESC& desc);
				void SetCommandQueueDescriptor(const D3D12_COMMAND_QUEUE_DESC& desc);
				uint32_t Create(ComPtr<ID3D12Device2>& device);
				const ComPtr<ID3D12CommandQueue>& GetCmdQueue() const { return m_CommandQueue; }
			private:
				ComPtr<ID3D12CommandQueue> m_CommandQueue;
				D3D12_COMMAND_QUEUE_DESC m_Desc;
			};

			class Allocator
			{
			public:
				Allocator();
				~Allocator() = default;
				void Allocate(const ComPtr<ID3D12Device2>& Device);
				const ComPtr<ID3D12CommandAllocator>& GetCommandAlloc() const { return m_CommandAllocator; }
			private:
				ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
			};
			class List
			{
			public:
				List() = default;
				~List() = default;
				void Close();
				void Create(const ComPtr<ID3D12Device2>& device, const ComPtr<ID3D12CommandAllocator>& cAlloc);
				const ComPtr<ID3D12GraphicsCommandList>& GetCommandList() const { return m_CommandList; }
			private:
				ComPtr<ID3D12GraphicsCommandList> m_CommandList;
				friend Allocator;
			};
		};
	}
}