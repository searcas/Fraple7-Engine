#include "pch.h"
#include "Renderer.h"
#include "Utilities/Common/Common.h"
#include "Graphics/3D Renderer/API/DirectX-12/SwapChain.h"
#include <DirectXMath.h>
namespace Fraple7
{
	namespace Core
	{
		// TODO://
		// Make proper Rendering Technique 
		// as
		// ExecuteIndirect
		// FrameBuffering sample
		// for number of backbuffers - >
		// https ://youtu.be/E3wTajGZOsA?si=HMrmKn0jrgJeZ_VW>
		// https ://jackmin.home.blog/2018/12/14/swapchains-present-and-present-latency/
		// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12ExecuteIndirect/src/DXSample.cpp
		Renderer::Renderer(Window& window)
			: m_Window((WinWindow&)window), 
			m_PipeLine(window),
			m_Projection(window), m_Texture(L"ShibaShitu.png", m_PipeLine.GetDevice()->GetDevice())
		{
			m_Fence = std::make_shared<FenceDx>(m_PipeLine.GetCommandQueue(), m_PipeLine.GetSwapChain()->GetBufferCount());
			m_Window.SetFenceRef(m_Fence);
			m_Fence->Create(m_PipeLine.GetDevice()->GetDevice());
			m_Fence->CreateAnEvent();
			m_VertexBuffer.Create(m_PipeLine.GetDevice()->GetDevice());

			auto& ComList  = m_PipeLine.GetCommandList().GetCommandList();
			auto& ComAlloc = m_PipeLine.GetCommandAllocator().GetCommandAlloc();
			auto& ComQueue = m_PipeLine.GetCommandQueue()->GetCmdQueue();
			CommandMgr que(ComList, ComAlloc, ComQueue);
			que.Join(m_VertexBuffer.GetVertexBuffer(), m_VertexBuffer.GetVertexUploadBuffer(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

			// insert fence to detect when upload is complete
			m_Fence->Complete();

			m_VertexBuffer.CreateVertexBufferView();
			m_IndexBuffer.Create(m_PipeLine.GetDevice()->GetDevice());

			que.Join(m_IndexBuffer.GetIndexBuffer(), m_IndexBuffer.GetIndexUploadBuffer(), D3D12_RESOURCE_STATE_INDEX_BUFFER);

			m_Fence->Complete();


			m_IndexBuffer.CreateIndexBufferView();

			m_Texture.Create();

			que.Join(m_Texture.GetTextureBuffer(), m_Texture.GetTextureUploadBuffer(), m_Texture.GetSubData().size(), m_Texture.GetSubData(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_Fence->Complete();

			m_Texture.DescriptorHeap();
			// Create descriptor in the heap
			m_Texture.ShaderResourceViewDesc();

			m_PSO.Create(m_PipeLine.GetDevice()->GetDevice());
			m_ScissorRect = CD3DX12_RECT{ 0,0, LONG_MAX, LONG_MAX };
			m_Viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, float(window.GetWidth()), float(window.GetHeight()) };
			m_Projection.SetView();
		}
	
		float t = 0.0f;
		float step = 0.01f;
		void Renderer::Render()
		{
			Update();
			m_CurrentBackBufferIndex = m_PipeLine.GetSwapChain()->GetSwapChain()->GetCurrentBackBufferIndex();

			auto& BackBuffer = m_PipeLine.GetSwapChain()->GetBackBuffer()[m_CurrentBackBufferIndex];

			auto& ComAll = m_PipeLine.GetCommandAllocator().GetCommandAlloc();
			ComAll->Reset() >> statusCode;
			auto& ComList = m_PipeLine.GetCommandList().GetCommandList();
			ComList->Reset(ComAll.Get(), nullptr) >> statusCode;

			const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{ m_PipeLine.GetSwapChain()->GetRTDescHeap()->GetCPUDescriptorHandleForHeapStart(),
														(INT)m_CurrentBackBufferIndex, m_PipeLine.GetSwapChain()->GetRenderTargetSize() };
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
			ComList->ClearDepthStencilView(m_PipeLine.GetDSVHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr );
			// Set Pipeline state
			ComList->SetPipelineState(m_PSO.GetPLS().Get());
			ComList->SetGraphicsRootSignature(m_PSO.GetRootSig().GetSignature().Get());

			// Configure IA
			ComList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			ComList->IASetVertexBuffers(0, 1, &m_VertexBuffer.GetVertexBufferView());
			ComList->IASetIndexBuffer(&m_IndexBuffer.GetIndexBufferView());

			ComList->RSSetViewports(1, &m_Viewport);
			ComList->RSSetScissorRects(1, &m_ScissorRect);

			//bind the heap containing the texture descriptor
			ComList->SetDescriptorHeaps(1, m_Texture.GetSrvHeap().GetAddressOf());
			ComList->SetGraphicsRootDescriptorTable(1, m_Texture.GetSrvHeap()->GetGPUDescriptorHandleForHeapStart());
			// bind render target & depth stencil
			ComList->OMSetRenderTargets(1, &rtv, TRUE, &m_PipeLine.GetDSVHandle());

			{
				const auto mvp = DirectX::XMMatrixTranspose(
					DirectX::XMMatrixRotationX(1.0f * t + 1.f) *
					DirectX::XMMatrixRotationY(1.2f * t + 2.f) *
					DirectX::XMMatrixRotationZ(1.1f * t + 0.f) *
					m_Projection.GetProjectionView());

				ComList->SetGraphicsRoot32BitConstants(0, sizeof(mvp) / 4, &mvp, 0);
				ComList->DrawIndexedInstanced(m_IndexBuffer.GetIndices(), 1, 0, 0, 0);
			}
			// Prepare buffer for presentation
			{
				const auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(BackBuffer.Get(),
					D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
				ComList->ResourceBarrier(1, &barrier);
			}
			// submit command list
			auto& cQueue = m_PipeLine.GetCommandQueue()->GetCmdQueue();
			
			{
				ComList->Close() >> statusCode;
				ID3D12CommandList* const CommandList[] = { ComList.Get() };
				cQueue->ExecuteCommandLists(
					_countof(CommandList), CommandList);
				
			}
			{
				// present frame
				m_PipeLine.GetSwapChain()->vSync();
				
				// insert fence to mark command list completion
				m_Fence->CompleteMultiFrame();
			}
			t += step;
		}


		void Renderer::Update()
		{
			static uint64_t frameCounter = 0;
			static double elapsedSeconds = 0.0;
			static std::chrono::high_resolution_clock clock;
			static auto t0 = clock.now();

			frameCounter++;
			auto t1 = clock.now();
			auto deltaTime = t1 - t0;
			t0 = t1;

			//Converting from nanoseconds to seconds 1 x 10^-9
			elapsedSeconds += deltaTime.count() * 1e-9;

			// Print every second only
			if (elapsedSeconds > 1.0)
			{
				char buffer[128];
				auto fps = frameCounter / elapsedSeconds;
				sprintf_s(buffer, 128, "FPS: %f\n", fps);
				OutputDebugString(buffer);

				frameCounter = 0;
				elapsedSeconds = 0.0;
			}
		}

	}
}
