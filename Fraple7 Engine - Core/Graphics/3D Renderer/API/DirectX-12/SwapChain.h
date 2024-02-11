#pragma once
#include "directx/d3dx12.h"
#include "Utilities/Common/Common.h"
#include "dxgi1_6.h"
#include "Command.h"

namespace Fraple7
{
	namespace Core
	{
		class Device;

		class SwapChain
		{
		public:
			SwapChain(std::shared_ptr<Studio::Window> window, std::shared_ptr<Device>, uint32_t bufferCount, const std::shared_ptr<Command::QueueDx>&);
			const ComPtr<IDXGISwapChain4>& GetSwapChain() const { return m_SwapChain4; }
		public:
			~SwapChain();
			void vSync(bool sync);
			uint32_t Create();
			uint32_t RenderTargetView();
			bool AllowTearing();
			uint32_t GetBufferCount() const { return m_BufferCount; }
			void SetBufferCount(uint32_t val) { m_BufferCount = val; }
			//std::vector<ComPtr<ID3D12Resource>>& GetBackBuffer() { return m_BackBuffers; }
			std::vector<ComPtr<ID3D12Resource>>& GetBackBuffer() { return m_BackBuffers; }
			const ComPtr<ID3D12DescriptorHeap>& GetRTDescHeap() const { return m_RtDescriptorHeap; }
			const UINT GetRenderTargetSize() const { return m_RenderTargetSize; }
			void ResizeSwapChain();
		private:
			std::shared_ptr<Device> m_Device;
			ComPtr<IDXGISwapChain1> m_SwapChain1;
			ComPtr<IDXGISwapChain4> m_SwapChain4;
			std::shared_ptr<Studio::WinWindow> m_Window;
			uint32_t m_BufferCount = 3;
			std::vector<ComPtr<ID3D12Resource>>m_BackBuffers;
			ComPtr<ID3D12DescriptorHeap>m_RtDescriptorHeap;
			UINT m_RenderTargetSize = 0;
			BOOL m_AllowTearing = false;
			std::shared_ptr<Command::QueueDx> m_CommandQueue;
		};
	}
}

