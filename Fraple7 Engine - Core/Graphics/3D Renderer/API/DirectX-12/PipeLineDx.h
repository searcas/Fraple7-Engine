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
			const Commands::Allocator& GetCommandAllocator() const { return m_CommandAllocator; }
			const Commands::List& GetCommandList() const { return m_CommandList; }
			const std::shared_ptr<Commands::QueueDx>& GetCommandQueue() const { return m_cQueue; }
			std::shared_ptr<Device>& GetDevice();
			std::shared_ptr<SwapChain>& GetSwapChain();
		public:

		private:
			uint32_t Commands() ;
		private:
			std::shared_ptr<Device>m_Device;
			std::shared_ptr<SwapChain> m_SwapChain;
			Commands::Allocator m_CommandAllocator;
			Commands::List m_CommandList;
			std::shared_ptr<Commands::QueueDx> m_cQueue;
			WinWindow& m_Window;
			std::unique_ptr<DepthBuffer> m_DepthBuffer;
		};

	}
}