#include "pch.h"
#include "PipeLineDx.h"
#include "Utilities/Common/Common.h"
#include "Studio/Platform/Windows/Window.h"

namespace Fraple7
{
	namespace Core
	{
		PipeLineDx::PipeLineDx(const Window& window, uint32_t BufferCount) : 
			m_Window(window), m_BufferCount(BufferCount)
		{
			Create();
			Commands();
		}
		PipeLineDx::~PipeLineDx()
		{
			Destroy();
		}
		uint32_t PipeLineDx::Create()
		{
			CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_DxGiFactory)) >> statusCode;
			D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_Device)) >> statusCode;
			const D3D12_COMMAND_QUEUE_DESC desc = {
			.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			.NodeMask = 0,
			};
			m_cQueue.SetCommandQueueDescriptor(desc);
			m_cQueue.Create(m_Device);
			SwapChain();
			RenderTargetView();

			return FPL_SUCCESS;
		}
		uint32_t PipeLineDx::Destroy()
		{
			return FPL_SUCCESS;
		}
		uint32_t PipeLineDx::SwapChain()
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
				.Flags = 0 //DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING
			};
			m_DxGiFactory->CreateSwapChainForHwnd(m_cQueue.GetCmdQueue().Get(), 
				WinWindow::GetHandle(), 
				&swapChainDesc, nullptr, nullptr, &m_SwapChain) >> statusCode;
			m_SwapChain2.As(&m_SwapChain) >> statusCode;

			Status = FPL_SUCCESS
			return Status;
		}
		uint32_t PipeLineDx::RenderTargetView()
		{
			uint32_t Status = FPL_PIPELINE_RENDER_TARGET_VIEW_ERROR;
			
			const D3D12_DESCRIPTOR_HEAP_DESC desc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
				.NumDescriptors = m_BufferCount,
			};
			m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_RtDescriptorHeap)) >> statusCode;

			m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			m_BackBuffers.reserve(m_BufferCount);

			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

				for (size_t i = 0; i < m_BufferCount; i++)
				{
					m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_BackBuffers[i])) >> statusCode;
					m_Device->CreateRenderTargetView(m_BackBuffers[i].Get(), nullptr, rtvHandle);
					rtvHandle.Offset(m_RtvDescriptorSize);
				}
				
			}

			Status = FPL_SUCCESS;
			return Status;
		}
		uint32_t PipeLineDx::Commands()
		{
			uint32_t Status = FPL_PIPELINE_RENDER_TARGET_VIEW_ERROR;
			m_CommandAllocator.Allocate(m_Device);
			m_CommandList.Create(m_Device, m_CommandAllocator.GetCommandAlloc());
			m_CommandList.Close();
			Status = FPL_SUCCESS;
			return Status;
		}
	}
}