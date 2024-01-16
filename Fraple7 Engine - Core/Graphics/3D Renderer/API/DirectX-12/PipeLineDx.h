#pragma once
#include "directx/d3dx12.h"
#include <wrl.h>
#include <dxgi1_6.h>

#include "Command.h"
#include "Graphics/3D Renderer/API/DirectX-12/DepthBuffer.h"
#include <vector>
#include "Device.h"
#include "SwapChain.h"
namespace Fraple7
{
	namespace Core
	{
		class PipeLineDx
		{
		private:
			
		public:
			PipeLineDx(class Window& window);
			PipeLineDx() = default;
			~PipeLineDx();
			uint32_t Create();
			const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetDSVHandle() { return m_DepthBuffer->GetDSVHandle(); }
		public:
			const std::shared_ptr<Command::QueueDx>& GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const;
			std::shared_ptr<Device>& GetDevice();
			std::shared_ptr<SwapChain>& GetSwapChain();
		private:
			std::shared_ptr<Device>m_Device;
			std::shared_ptr<SwapChain> m_SwapChain;
			std::shared_ptr<Command::QueueDx> m_CommandQueueCopy;
			std::shared_ptr<Command::QueueDx> m_CommandQueueDirect;
			std::shared_ptr<Command::QueueDx> m_CommandQueueCompute;
			WinWindow& m_Window;
			std::unique_ptr<DepthBuffer> m_DepthBuffer;
		};

	}
}