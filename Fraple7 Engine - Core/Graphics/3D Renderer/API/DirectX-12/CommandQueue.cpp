#include "pch.h"
#include "CommandQueue.h"
#include "Utilities/Common/Common.h"

namespace Fraple7
{
	namespace Core
	{

		CommQueue::CommQueue(const D3D12_COMMAND_QUEUE_DESC& desc)
			: m_Desc(desc)
		{

		}

		void CommQueue::SetCommandQueueDescriptor(const D3D12_COMMAND_QUEUE_DESC& desc)
		{
			m_Desc = desc;
		}

		uint32_t CommQueue::Create(ComPtr<ID3D12Device2>& device)
		{
			device->CreateCommandQueue(&m_Desc, IID_PPV_ARGS(&m_CommandQueue)) >> statusCode;
			return FPL_SUCCESS;
		}

	}
}