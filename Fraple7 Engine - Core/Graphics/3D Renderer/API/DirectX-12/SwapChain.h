#pragma once
#include "directx/d3dx12.h"
#include "Utilities/Common/Common.h"
#include "dxgi1_6.h"
#include "Command.h"
namespace Fraple7
{
	namespace Core
	{
		class SwapChain
		{
		public:
			SwapChain(class Window& window, std::shared_ptr<class  Device>&, uint32_t bufferCount);
			const ComPtr<IDXGISwapChain4>& GetSwapChain() const { return m_SwapChain4; }
		public:
			~SwapChain();
			void Sync(uint8_t interval, uint32_t flags);
			uint32_t Create(std::shared_ptr<Commands::QueueDx>& cQueue);
			uint32_t RenderTargetView();

			uint32_t GetBufferCount() const { return m_BufferCount; }
			void SetBufferCount(uint32_t val) { m_BufferCount = val; }
			std::vector<ComPtr<ID3D12Resource>>& GetBackBuffer() { return m_BackBuffers; }
			const ComPtr<ID3D12DescriptorHeap>& GetRTDescHeap() const { return m_RtDescriptorHeap; }
			const UINT GetRenderTargetSize() { return m_RenderTargetSize; }
		private:
			std::shared_ptr<class Device>& m_Device;
			ComPtr<IDXGISwapChain1> m_SwapChain1;
			ComPtr<IDXGISwapChain4> m_SwapChain4;
			class WinWindow& m_Window;
			uint32_t m_BufferCount = 3;
			std::vector<ComPtr<ID3D12Resource>>m_BackBuffers;
			ComPtr<ID3D12DescriptorHeap>m_RtDescriptorHeap;
			UINT m_RenderTargetSize = 0;
		};
	}
}

