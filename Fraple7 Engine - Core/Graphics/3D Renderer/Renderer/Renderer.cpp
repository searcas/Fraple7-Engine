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
			m_VertexBuffer.Create(m_PipeLine.GetDevice());
			auto& ComList = m_PipeLine.GetCommandList().GetCommandList();
			auto& ComAlloc = m_PipeLine.GetCommandAllocator().GetCommandAlloc();
			auto& ComQueue = m_PipeLine.GetCommandQueue().GetCmdQueue();
			ComAlloc->Reset() >> statusCode;
			ComList->Reset(ComAlloc.Get(), nullptr) >> statusCode;
			ComList->CopyResource(m_VertexBuffer.GetVertexBuffer().Get(), m_VertexBuffer.GetVertexUploadBuffer().Get());
			ComList->Close() >> statusCode;

			// Submit command list to queue as array with single element
			ID3D12CommandList* const commandLists[] = { ComList.Get() };
			ComQueue->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

			// insert fence to detect when upload is complete
			m_Fence.Signal();
			m_Fence.Wait(INFINITE);

			m_VertexBuffer.CreateVertexBufferView();

			m_PSO.Create(m_PipeLine.GetDevice());

			m_ScissorRect = CD3DX12_RECT{ 0,0, LONG_MAX, LONG_MAX };
			m_Viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, float(window.GetWidth()), float(window.GetHeight()) };
		}
		float t = 0.0f;
		float step = 0.001f;
		void Renderer::Render()
		{
			m_CurrentBackBufferIndex = m_PipeLine.GetSwapChain().GetSwapChain()->GetCurrentBackBufferIndex();

			auto& BackBuffer = m_PipeLine.GetBackBuffer()[m_CurrentBackBufferIndex];

			auto& ComAll = m_PipeLine.GetCommandAllocator().GetCommandAlloc();
			ComAll->Reset() >> statusCode;
			auto& ComList = m_PipeLine.GetCommandList().GetCommandList();
			ComList->Reset(ComAll.Get(), nullptr) >> statusCode;

			const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{ m_PipeLine.GetRTDescHeap()->GetCPUDescriptorHandleForHeapStart(),
														(INT)m_CurrentBackBufferIndex,m_PipeLine.GetRtDescSize() };
			{
				// transition buffer resource to render target state
				const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(BackBuffer.Get(), 
					D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
				ComList->ResourceBarrier(1, &barrier);

				FLOAT clearColor[] = { 
					
					sin(2.f * t + 1.f) / 2.f + .5f,
					sin(3.f * t + 3.f) / 2.f + .5f,
					sin(5.f * t + 2.f) / 2.f + .5f,
					1.0f };

				ComList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
			}
			// Set Pipeline state
			ComList->SetPipelineState(m_PSO.GetPLS().Get());
			ComList->SetGraphicsRootSignature(m_PSO.GetRootSig().GetSignature().Get());

			// Configure IA
			ComList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			ComList->IASetVertexBuffers(0, 1, &m_VertexBuffer.GetVertexBufferView());

			ComList->RSSetViewports(1, &m_Viewport);
			ComList->RSSetScissorRects(1, &m_ScissorRect);

			// bind render target
			ComList->OMSetRenderTargets(1, &rtv, TRUE, nullptr);
			ComList->DrawInstanced(m_VertexBuffer.GetNumVertices(), 1, 0, 0);
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
			t += step;
		}

	}
}
