#pragma once
#include "../PipeLine/PipeLine.h"
#include <wrl.h>
#include <dxgi1_6.h>
#include "directx/d3d12.h"
#include "CommandQueue.h"

namespace Fraple7
{
	namespace Core
	{
		class PipeLineDx : public PipeLine
		{
		public:
			PipeLineDx(const class Window& window, uint32_t BufferCount);
			~PipeLineDx();
		public:
			void SetBufferCount(uint32_t val) { m_BufferCount = val; }
			uint32_t GetBufferCount() const { return m_BufferCount; }
		private:
			uint32_t Create() override;
			uint32_t Destroy() override;
			uint32_t SwapChain() override;
			uint32_t RenderTargetView() override;
		private:
			Microsoft::WRL::ComPtr<IDXGIFactory4> m_DxGiFactory;
			Microsoft::WRL::ComPtr<ID3D12Device2> m_Device;
			CommQueue m_cQueue;
			Microsoft::WRL::ComPtr<IDXGISwapChain1> m_SwapChain;
			Microsoft::WRL::ComPtr<IDXGISwapChain4> m_SwapChain2;
			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>m_RtDescriptorHeap;

			const Window& m_Window;
			uint32_t m_BufferCount;
		};

	}
}