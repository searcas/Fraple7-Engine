#include "pch.h"
#include "Renderer.h"
#include "Studio/Platform/Abstract/Window.h"
#include "Utilities/Common/Common.h"
#include "Graphics/3D Renderer/API/DirectX-12/VertexBuffer.h"

namespace Fraple7
{
	namespace Core
	{
		Renderer::Renderer(const Window& window) 
			: m_PipeLine(window, 2), m_Fence(m_PipeLine.GetCommandQueue())
		{
			m_Fence.Create(m_PipeLine.GetDevice());
			m_Fence.Signaling();
			VertexBuffer vertexBuffer(m_PipeLine.GetDevice());
			auto& ComList = m_PipeLine.GetCommandList().GetCommandList();
			auto& ComAlloc = m_PipeLine.GetCommandAllocator().GetCommandAlloc();
			auto& ComQueue = m_PipeLine.GetCommandQueue().GetCmdQueue();
			ComAlloc->Reset() >> statusCode;
			ComList->Reset(ComAlloc.Get(), nullptr) >> statusCode;
			ComList->CopyResource(vertexBuffer.GetVertexBuffer().Get(), vertexBuffer.GetVertexUploadBuffer().Get());
			ComList->Close() >> statusCode;

			// Submit command list to queue as array with single element
			ID3D12CommandList* const commandLists[] = { ComList.Get() };
			ComQueue->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

			// insert fence to detect when upload is complete
			m_Fence.Signal();
			m_Fence.Wait(INFINITE);

			vertexBuffer.CreateVertexBufferView();
			
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

				FLOAT clearColor[] = { 0.9f, 0.69f, 0.69f, 1.0f };

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
				m_Fence.Signal();
				// present frame
				m_PipeLine.GetSwapChain().Sync(0,0);

				//wait for command list / allocator to become free
				m_Fence.Wait(INFINITE);
			}
		}

	}
}
