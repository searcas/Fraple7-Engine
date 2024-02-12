#pragma once
#include "directx/d3d12.h"

namespace Fraple7
{
	namespace Core
	{
		class CommandQueue;
		class CommandMgr
		{
		public:
			CommandMgr();
			~CommandMgr();
			const std::shared_ptr<CommandQueue>& GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const;
			void UnloadAll();
		private:
			std::shared_ptr<CommandQueue> m_CommandQueueCopy;
			std::shared_ptr<CommandQueue> m_CommandQueueDirect;
			std::shared_ptr<CommandQueue> m_CommandQueueCompute;
		};
	}
}