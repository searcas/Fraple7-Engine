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
	/*	class Device;
		class SwapChain;*/

		class PipeLineDx
		{
		private:
			
		public:
			PipeLineDx(class Window& window);
			PipeLineDx() = default;
			~PipeLineDx();
			uint32_t Create();
			const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetDSVHandle() { return m_DepthBuffer.GetDSVHandle(); }
		public:
			const Commands::Allocator& GetCommandAllocator() const { return m_CommandAllocator; }
			const Commands::List& GetCommandList() const { return m_CommandList; }
			const Commands::QueueDx& GetCommandQueue() const { return m_cQueue; }
			const UINT& GetRtDescSize() { return m_RtvDescriptorSize; }
			Device& GetDevice(); 
			SwapChain& GetSwapChain();
		public:

		private:
			uint32_t Commands() ;
		private:
			Device m_Device;
			SwapChain m_SwapChain;
			UINT m_RtvDescriptorSize;
			Commands::Allocator m_CommandAllocator;
			Commands::List m_CommandList;
			Commands::QueueDx m_cQueue;
			Window& m_Window;
			DepthBuffer m_DepthBuffer;
		};

	}
}