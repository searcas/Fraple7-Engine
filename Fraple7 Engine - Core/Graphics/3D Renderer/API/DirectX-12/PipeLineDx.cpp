#include "pch.h"
#include "PipeLineDx.h"
#include "Utilities/Common/Common.h"
#include "Studio/Platform/Windows/Window.h"

namespace Fraple7
{
	namespace Core
	{

		PipeLineDx::PipeLineDx(Window& window) : 
			m_Window((WinWindow&)window)
		{
			m_Device = std::make_shared<Device>();
			m_DepthBuffer = std::make_unique<DepthBuffer>(m_Device->GetDevice(), m_Window);
			m_SwapChain = std::make_shared<SwapChain>(window, m_Device, 3);
		
			m_CommandQueueCopy = std::make_shared<Command::QueueDx>(m_Device->GetDevice(), D3D12_COMMAND_LIST_TYPE_COPY); 
			m_CommandQueueDirect = std::make_shared<Command::QueueDx>(m_Device->GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);
			m_CommandQueueCompute = std::make_shared<Command::QueueDx>(m_Device->GetDevice(), D3D12_COMMAND_LIST_TYPE_COMPUTE);

			m_Window.SetDeviceRef(m_Device);
			m_Window.SetSwapChainRef(m_SwapChain);
			m_Window.SetCommandQueueRef(m_CommandQueueDirect);

			Create();
			m_DepthBuffer->Init();
		}
		PipeLineDx::~PipeLineDx()
		{
		}
		uint32_t PipeLineDx::Create()
		{
			m_SwapChain->Create(m_CommandQueueDirect);
			m_SwapChain->RenderTargetView();
			return FPL_SUCCESS;
		}
		
		const std::shared_ptr<Command::QueueDx>& PipeLineDx::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
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
				throw std::runtime_error{"Failed Command Queue Type"};
				break;
			}
		}

		std::shared_ptr<Device>& PipeLineDx::GetDevice()
		{
			return m_Device;
		}

		std::shared_ptr<SwapChain>& PipeLineDx::GetSwapChain()
		{
			return m_SwapChain;
		}
	}
}