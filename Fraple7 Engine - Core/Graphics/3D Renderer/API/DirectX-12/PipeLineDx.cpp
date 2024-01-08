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
			m_SwapChain= std::make_shared<SwapChain>(window, m_Device, 3);
			m_cQueue= std::make_shared<Commands::QueueDx>();

			m_Window.SetDeviceRef(m_Device);
			m_Window.SetSwapChainRef(m_SwapChain);
			m_Window.SetCQueueRef(m_cQueue);

			Create();
			m_DepthBuffer->Init();
			Commands();
		}
		PipeLineDx::~PipeLineDx()
		{
		}
		uint32_t PipeLineDx::Create()
		{
			m_cQueue->SetCommandQueueDescriptor(m_cQueue->SetDescriptionDirectNormal());
			m_cQueue->Create(m_Device->GetDevice());
			m_SwapChain->Create(m_cQueue);
			m_SwapChain->RenderTargetView();
			return FPL_SUCCESS;
		}
		
		std::shared_ptr<Device>& PipeLineDx::GetDevice()
		{
			return m_Device;
		}

		std::shared_ptr<SwapChain>& PipeLineDx::GetSwapChain()
		{
			return m_SwapChain;
		}

		uint32_t PipeLineDx::Commands()
		{
			uint32_t Status = FPL_PIPELINE_RENDER_TARGET_VIEW_ERROR;
			m_CommandAllocator.Allocate(m_Device->GetDevice());
			m_CommandList.Create(m_Device->GetDevice(), m_CommandAllocator.GetCommandAlloc());
			m_CommandList.Close();
			Status = FPL_SUCCESS;
			return Status;
		}
	}
}