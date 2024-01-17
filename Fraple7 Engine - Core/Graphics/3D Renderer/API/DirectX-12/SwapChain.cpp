#include "pch.h"
#include "SwapChain.h"
#include "Device.h"
#ifdef WINDOWS
#include "Studio/Platform/Windows/Window.h"
#endif

namespace Fraple7
{
	namespace Core
	{
		uint32_t SwapChain::RenderTargetView()
		{
			uint32_t Status = FPL_PIPELINE_RENDER_TARGET_VIEW_ERROR;

			const D3D12_DESCRIPTOR_HEAP_DESC desc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
				.NumDescriptors = m_BufferCount,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE
			};
			m_Device->Construct();
			m_Device->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_RtDescriptorHeap)) >> statusCode;
			 
			m_RenderTargetSize = m_Device->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			m_BackBuffers.resize(m_BufferCount);

			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

				for (size_t i = 0; i < m_BufferCount; i++)
				{
					ComPtr<ID3D12Resource> backbuffer;
					m_SwapChain4->GetBuffer(i, IID_PPV_ARGS(&backbuffer)) >> statusCode;
					m_Device->GetDevice()->CreateRenderTargetView(backbuffer.Get(), nullptr, rtvHandle);
					m_BackBuffers[i] = backbuffer;
					rtvHandle.Offset(m_RenderTargetSize);
				}

			}

			Status = FPL_SUCCESS;
			return Status;
		}
		bool SwapChain::AllowTearing()
		{

			ComPtr<IDXGIFactory4> factory4;
			if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
			{
				ComPtr<IDXGIFactory5> factory5;
				if (SUCCEEDED(factory4.As(&factory5)))
				{
					if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_AllowTearing, sizeof(m_AllowTearing)))) {
						return m_AllowTearing;
					}
					else return m_AllowTearing;
				}
			}
			
			return m_AllowTearing;
		}
		uint32_t SwapChain::Create()
		{
			uint32_t Status = FPL_PIPELINE_SWAP_CHAIN_ERROR;
			const DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
				.Width = m_Window.GetWidth(),
				.Height = m_Window.GetHeight(),
				.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
				.Stereo = FALSE,
				.SampleDesc = {
					.Count = 1,
					.Quality = 0
				},
				.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
				.BufferCount = m_BufferCount,
				.Scaling = DXGI_SCALING_STRETCH,
				.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
				.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
				.Flags = (AllowTearing() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u)
			};
			m_Device->GetDXGIFactory()->CreateSwapChainForHwnd(m_CommandQueue->GetCmdQueue().Get(),
				m_Window.GetHandle(),
				&swapChainDesc, nullptr, nullptr, &m_SwapChain1) >> statusCode;
			m_Device->GetDXGIFactory()->MakeWindowAssociation(m_Window.GetHandle(), DXGI_MWA_NO_ALT_ENTER) >> statusCode;
			m_SwapChain1.As(&m_SwapChain4) >> statusCode;

			Status = FPL_SUCCESS
			return Status;
		}
		SwapChain::SwapChain(Window& window, std::shared_ptr<Device>& device, uint32_t bufferCount, const std::shared_ptr<Command::QueueDx>& commandQueue)
			: m_Window((WinWindow&)window), m_Device(device), m_BufferCount(bufferCount), m_CommandQueue(commandQueue)
		{
			m_BackBuffers.resize(bufferCount);
		}
		SwapChain::~SwapChain()
		{
		}
		void SwapChain::vSync()
		{
			uint16_t interval, flags;
			interval = m_vSync;
			flags = m_AllowTearing && !m_vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
			m_SwapChain4->Present(interval, flags) >> statusCode;
		}
		void SwapChain::ResizeSwapChain()
		{
			m_Window.SetWidth(std::max(1u, m_Window.GetWidth()));
			m_Window.SetHeight(std::max(1u, m_Window.GetHeight()));
			if (m_Device.get() == nullptr)
				return;

			m_CommandQueue->SignalAndWait();
			for (size_t i = 0; i < m_BufferCount; i++)
			{
				m_BackBuffers[i].Reset();
			}
			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			m_SwapChain4->GetDesc(&swapChainDesc) >> statusCode;
			m_SwapChain4->ResizeBuffers(m_BufferCount, m_Window.GetWidth(), m_Window.GetHeight(),
				swapChainDesc.BufferDesc.Format, swapChainDesc.Flags) >> statusCode;

			RenderTargetView();
		}
	}
}