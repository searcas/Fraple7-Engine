#include "pch.h"
#include "Command.h"
#include "Utilities/Common/Common.h"

namespace Fraple7
{
	namespace Core
	{

		Commands::QueueDx::QueueDx(const D3D12_COMMAND_QUEUE_DESC& desc)
			: m_Desc(desc)
		{

		}

		void Commands::QueueDx::SetCommandQueueDescriptor(const D3D12_COMMAND_QUEUE_DESC& desc)
		{
			m_Desc = desc;
		}

		uint32_t Commands::QueueDx::Create(const ComPtr<ID3D12Device2>& device)
		{
			device->CreateCommandQueue(&m_Desc, IID_PPV_ARGS(&m_CommandQueue)) >> statusCode;
			return FPL_SUCCESS;
		}
		D3D12_COMMAND_QUEUE_DESC Commands::QueueDx::SetCustomDescription(D3D12_COMMAND_LIST_TYPE type, 
			INT priority, D3D12_COMMAND_QUEUE_FLAGS flags, UINT NodeMask) const
		{
			const D3D12_COMMAND_QUEUE_DESC desc = {
				.Type = type,
				.Priority = priority,
				.Flags = flags,
				.NodeMask = NodeMask,
			};
			return desc;
		}
		D3D12_COMMAND_QUEUE_DESC Commands::QueueDx::SetDescriptionDirectNormal() const
		{
			const D3D12_COMMAND_QUEUE_DESC desc = {
				.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
				.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
				.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
				.NodeMask = 0,
			};
			return desc;
		}

		Commands::Allocator::Allocator()
		{
			
		}

		void Commands::Allocator::Allocate(const ComPtr<ID3D12Device2>& device)
		{
			device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
		}

		void Commands::List::Close()
		{
			m_CommandList->Close() >> statusCode;
		}

		void Commands::List::Create(const ComPtr<ID3D12Device2>& device, const ComPtr<ID3D12CommandAllocator>& cAlloc)
		{
			device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
			cAlloc.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)) >> statusCode;
		}
		void CommandMgr::Join(const ComPtr<ID3D12Resource>& dst, const ComPtr<ID3D12Resource>& src, D3D12_RESOURCE_STATES TransitionState)
		{
			m_ComAll->Reset() >> statusCode;
			m_ComList->Reset(m_ComAll.Get(), nullptr) >> statusCode;
			m_ComList->CopyResource(dst.Get(), src.Get());
			m_ComList->Close() >> statusCode;

			// Submit command list to queue as array with single element
			ID3D12CommandList* const commandLists[] = { m_ComList.Get() };
			m_ComQ->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

		}
		void CommandMgr::Join(const ComPtr<ID3D12Resource>& dst, const ComPtr<ID3D12Resource>& src, size_t size, const std::vector<D3D12_SUBRESOURCE_DATA>& srcData, D3D12_RESOURCE_STATES TransitionState)
		{
			m_ComAll->Reset() >> statusCode;
			m_ComList->Reset(m_ComAll.Get(), nullptr) >> statusCode;
			UpdateSubresources(m_ComList.Get(), dst.Get(), src.Get(), 0, 0, (UINT)size, srcData.data());
			Transition(dst, TransitionState);
			m_ComList->Close() >> statusCode;

			// Submit command list to queue as array with single element
			ID3D12CommandList* const commandLists[] = { m_ComList.Get() };
			m_ComQ->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);
		}
		void CommandMgr::Transition(const ComPtr<ID3D12Resource>& buffer, D3D12_RESOURCE_STATES state)
		{
			const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(buffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, state);
			m_ComList->ResourceBarrier(1, &barrier);
		}
		CommandMgr::CommandMgr(const ComPtr<ID3D12GraphicsCommandList>& comList, const ComPtr<ID3D12CommandAllocator>& comAll, const ComPtr<ID3D12CommandQueue>& comQ)
			: m_ComList(comList), m_ComAll(comAll), m_ComQ(comQ)
		{ 

		}
		
	}
}