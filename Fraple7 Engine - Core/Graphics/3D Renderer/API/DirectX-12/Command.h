#pragma once
#include "directx/d3dx12.h"
#include "Utilities/Common/Common.h"
namespace Fraple7
{
	namespace Core
	{
		class CommandMgr
		{
		public:
			CommandMgr(const ComPtr<ID3D12GraphicsCommandList>&, const ComPtr<ID3D12CommandAllocator>&, const ComPtr<ID3D12CommandQueue>&);
			void Join(const ComPtr<ID3D12Resource>& dst, const ComPtr<ID3D12Resource>& src, D3D12_RESOURCE_STATES);
			void Join(const ComPtr<ID3D12Resource>& dst, const ComPtr<ID3D12Resource>& src, size_t size, const std::vector<D3D12_SUBRESOURCE_DATA>& srcData, D3D12_RESOURCE_STATES);
			void Transition(const ComPtr<ID3D12Resource>& buffer, D3D12_RESOURCE_STATES state);
			~CommandMgr() = default;
		private:
			const ComPtr<ID3D12GraphicsCommandList>& m_ComList;
			const ComPtr<ID3D12CommandAllocator>& m_ComAll;
			const ComPtr<ID3D12CommandQueue>& m_ComQ;
		};
		class Commands
		{
		public:
			class QueueDx
			{
			public:
				QueueDx() = default;
				QueueDx(const D3D12_COMMAND_QUEUE_DESC& desc);
				uint32_t Create(const ComPtr<ID3D12Device2>& device);
				const ComPtr<ID3D12CommandQueue>& GetCmdQueue() const { return m_CommandQueue; }
				
				void SetCommandQueueDescriptor(const D3D12_COMMAND_QUEUE_DESC& desc);
				D3D12_COMMAND_QUEUE_DESC SetDescriptionDirectNormal() const;
				D3D12_COMMAND_QUEUE_DESC SetCustomDescription(D3D12_COMMAND_LIST_TYPE type,
					INT priority, D3D12_COMMAND_QUEUE_FLAGS flags, UINT NodeMask) const;

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