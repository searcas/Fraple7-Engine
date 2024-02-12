#include "pch.h"
#include "CommandMgr.h"
#include "CommandQueue.h"

namespace Fraple7
{
	namespace Core
	{

		CommandMgr::CommandMgr()
		{
			m_CommandQueueCopy = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COPY);
			m_CommandQueueDirect = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT);
			m_CommandQueueCompute = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		}

		CommandMgr::~CommandMgr()
		{
			UnloadAll();
		}
		const std::shared_ptr<CommandQueue>& CommandMgr::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
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
	}
}