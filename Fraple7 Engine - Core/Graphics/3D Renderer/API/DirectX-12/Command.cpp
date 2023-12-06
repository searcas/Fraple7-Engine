#include "pch.h"
#include "Command.h"
#include "Utilities/Common/Common.h"

namespace Fraple7
{
	namespace Core
	{

		Commands::Queue::Queue(const D3D12_COMMAND_QUEUE_DESC& desc)
			: m_Desc(desc)
		{

		}

		void Commands::Queue::SetCommandQueueDescriptor(const D3D12_COMMAND_QUEUE_DESC& desc)
		{
			m_Desc = desc;
		}

		uint32_t Commands::Queue::Create(ComPtr<ID3D12Device2>& device)
		{
			device->CreateCommandQueue(&m_Desc, IID_PPV_ARGS(&m_CommandQueue)) >> statusCode;
			return FPL_SUCCESS;
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
	}
}