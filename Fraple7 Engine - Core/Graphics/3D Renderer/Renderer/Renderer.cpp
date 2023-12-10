#include "pch.h"
#include "Renderer.h"
#include "Studio/Platform/Abstract/Window.h"
#include "Utilities/Common/Common.h"
namespace Fraple7
{
	namespace Core
	{
		Renderer::Renderer(const Window& window) 
			: m_PipeLine(window, 2), m_Fence(m_PipeLine.GetCommandQueue())
		{
			m_Fence.Create(m_PipeLine.GetDevice());
			m_Fence.Signaling();
		}

		void Renderer::Render()
		{
			m_CurrentBackBufferIndex = m_PipeLine.GetSwapChain().GetSwapChain()->GetCurrentBackBufferIndex();

			auto& BackBuffer = m_PipeLine.GetBackBuffer()[m_CurrentBackBufferIndex];

			auto& ComAll = m_PipeLine.GetCommandAllocator().GetCommandAlloc();
			ComAll->Reset() >> statusCode;
			auto& ComList = m_PipeLine.GetCommandList().GetCommandList();
			ComList->Reset(ComAll.Get(), nullptr) >> statusCode;

			{
				// transition buffer resource to render target state
				const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(BackBuffer.Get(), 
					D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
				ComList->ResourceBarrier(1, &barrier);

				FLOAT clearColor[] = { 0.4f, 0.1f, 0.69f, 1.0f };

				const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{ m_PipeLine.GetRTDescHeap()->GetCPUDescriptorHandleForHeapStart(),
															(INT)m_CurrentBackBufferIndex,m_PipeLine.GetRtDescSize() };
				ComList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
			}
			// Prepare buffer for presentation
			{
				const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(BackBuffer.Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
				ComList->ResourceBarrier(1, &barrier);
			}
			// submit command list
			auto& cQueue = m_PipeLine.GetCommandQueue().GetCmdQueue();
			{
				ComList->Close() >> statusCode;
				ID3D12CommandList* const CommandList[] = { ComList.Get() };
				cQueue->ExecuteCommandLists(
					_countof(CommandList), CommandList);
				
			}
			{
				// insert fence to mark command list completion
				auto& fenceVal = m_Fence.GetFenceVal();
				cQueue->Signal(m_Fence.GetFence().Get(), fenceVal++) >> statusCode;
				// present frame
				m_PipeLine.GetSwapChain().Sync(0,0);

				//wait for command list / allocator to become free
				m_Fence.GetFence()->SetEventOnCompletion(fenceVal - 1, m_Fence.GetFenceEvent()) >> statusCode;
				if (::WaitForSingleObject(m_Fence.GetFenceEvent(), INFINITE) == WAIT_FAILED)
				{
					GetLastError() >> statusCode;
				}

			}
		}

	}
}
