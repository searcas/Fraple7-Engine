#pragma once
#include "../PipeLine/PipeLine.h"
#include "directx/d3dx12.h"
#include <wrl.h>
#include <dxgi1_6.h>
#include "Command.h"
#include <vector>
namespace Fraple7
{
	namespace Core
	{
		class PipeLineDx : public PipeLine
		{
		public:
			PipeLineDx(const class Window& window, uint32_t BufferCount);
			~PipeLineDx();
			uint32_t Create() override;
		public:
			void SetBufferCount(uint32_t val) { m_BufferCount = val; }
		public:
			uint32_t GetBufferCount() const { return m_BufferCount; }
			const ComPtr<IDXGISwapChain4>& GetSwapChain() const { return m_SwapChain2; }
			const Commands::Allocator& GetCommandAllocator() const { return m_CommandAllocator; }
			const Commands::List& GetCommandList() const { return m_CommandList; }
			const Commands::Queue& GetCommandQueue() const { return m_cQueue; }

			const std::vector<ComPtr<ID3D12Resource>>& GetBackBuffer() { return m_BackBuffers; }
			const ComPtr<ID3D12Device2>& GetDevice() { return m_Device; }
			const ComPtr<ID3D12DescriptorHeap>& GetRTDescHeap() { return m_RtDescriptorHeap; }
			const UINT& GetRtDescSize() { return m_RtvDescriptorSize; }

		private:
			uint32_t Destroy() override;
			uint32_t SwapChain() override;
			uint32_t RenderTargetView() override;
			uint32_t Commands() override;
		private:
			ComPtr<IDXGIFactory4> m_DxGiFactory;
			ComPtr<ID3D12Device2> m_Device;
			ComPtr<IDXGISwapChain1> m_SwapChain;
			ComPtr<IDXGISwapChain4> m_SwapChain2;
			ComPtr<ID3D12DescriptorHeap>m_RtDescriptorHeap;
			UINT m_RtvDescriptorSize;
			std::vector<ComPtr<ID3D12Resource>>m_BackBuffers;
			Commands::Allocator m_CommandAllocator;
			Commands::List m_CommandList;
			Commands::Queue m_cQueue;
			uint32_t m_BufferCount;
			const Window& m_Window;
		};

	}
}