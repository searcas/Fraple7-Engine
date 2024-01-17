#include "pch.h"
#include "PipeLineDx.h"
#include "Utilities/Common/Common.h"
#include "Studio/Platform/Windows/Window.h"

namespace Fraple7
{
	namespace Core
	{

		PipeLineDx::PipeLineDx(Window& window) : 
			m_Window((WinWindow&)window)
		{
			m_Device = std::make_shared<Device>();

			m_CommandMgr = std::make_shared<CommandMgr>(m_Device->GetDevice());

			m_DepthBuffer = std::make_shared<DepthBuffer>(m_Device->GetDevice(), m_Window, m_CommandMgr);

			m_SwapChain = std::make_shared<SwapChain>(window, m_Device, 3, m_CommandMgr->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));

			m_Texture = std::make_unique<Texture>(L"ShibaShitu.png", m_Device->GetDevice());
			m_PSO = std::make_unique<PSO>(m_Device->GetDevice());
			m_VertexBuffer = std::make_unique<VertexBuffer>();
			m_IndexBuffer = std::make_unique<IndexBuffer>();

			m_Window.SetSwapChainRef(m_SwapChain);
			m_Window.SetDepthBufferRef(m_DepthBuffer);


			m_Projection = std::make_unique<Projection>(window);

			m_FenceValues.resize(m_SwapChain->GetBufferCount());

		}
		PipeLineDx::~PipeLineDx()
		{
			m_CommandMgr->UnloadAll();
		}

		std::shared_ptr<Device>& PipeLineDx::GetDevice()
		{
			return m_Device;
		}

		std::shared_ptr<SwapChain>& PipeLineDx::GetSwapChain()
		{
			return m_SwapChain;
		}
		void PipeLineDx::CleanCommandQueue()
		{
			m_CommandMgr->UnloadAll();
		}
		void PipeLineDx::Init()
		{
			m_SwapChain->Create();
			m_SwapChain->RenderTargetView();
			m_DepthBuffer->Init();

			m_VertexBuffer->Create(m_Device->GetDevice());

			const auto& ComQueue = m_CommandMgr->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);

			ComQueue->Join(m_VertexBuffer->GetVertexBuffer(), m_VertexBuffer->GetVertexUploadBuffer());
			// insert fence to detect when upload is complete
			ComQueue->SignalAndWait();

			m_VertexBuffer->CreateVertexBufferView();
			m_IndexBuffer->Create(m_Device->GetDevice());

			ComQueue->Join(m_IndexBuffer->GetIndexBuffer(), m_IndexBuffer->GetIndexUploadBuffer());
			ComQueue->SignalAndWait();

			m_IndexBuffer->CreateIndexBufferView();
			m_Texture->Create();

			ComQueue->Join(m_Texture->GetTextureBuffer(), m_Texture->GetTextureUploadBuffer(), m_Texture->GetSubData().size(), m_Texture->GetSubData());
			ComQueue->SignalAndWait();

			m_Texture->DescriptorHeap();
			// Create descriptor in the heap
			m_Texture->ShaderResourceViewDesc();

			m_PSO->Create();

			// Ensures that scrissor rectangle covers the entire screen 
			// regardless of the size of the screen
			m_ScissorRect = CD3DX12_RECT{ 0,0, LONG_MAX, LONG_MAX };
			// specifices the viewable part of the screen to render to
			m_Viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, float(m_Window.GetWidth()), float(m_Window.GetHeight()) };
			m_Projection->SetView();

			ComQueue->WaitForFenceCompletion(ComQueue->ExecuteCommandList(ComQueue->GetCommandList()));
			m_InitComplete = true;
			m_DepthBuffer->SetInitComplete(m_InitComplete);

		}
		float t = 0.0f;
		float step = 0.01f;
		void PipeLineDx::Render()
		{

			m_CurrentBackBufferIndex = m_SwapChain->GetSwapChain()->GetCurrentBackBufferIndex();

			auto& BackBuffer = m_SwapChain->GetBackBuffer()[m_CurrentBackBufferIndex];

			auto& CommandQueue = m_CommandMgr->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
			const auto& CommandList = CommandQueue->GetCommandList();

			const CD3DX12_CPU_DESCRIPTOR_HANDLE rtv{ m_SwapChain->GetRTDescHeap()->GetCPUDescriptorHandleForHeapStart(),
														(INT)m_CurrentBackBufferIndex,m_SwapChain->GetRenderTargetSize() };
			{
				// transition buffer resource to render target state
				CommandQueue->Transition(BackBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

				FLOAT clearColor[] = {

					sin(2.f * t + 1.f) / 2.f + .5f,
					sin(3.f * t + 3.f) / 2.f + .5f,
					sin(5.f * t + 2.f) / 2.f + .5f,
					1.0f };

				CommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
			}
			CommandList->ClearDepthStencilView(GetDSVHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
			// Set Pipeline state
			CommandList->SetPipelineState(m_PSO->GetPLS().Get());
			CommandList->SetGraphicsRootSignature(m_PSO->GetRootSig().GetSignature().Get());

			// Configure IA
			CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			CommandList->IASetVertexBuffers(0, 1, &m_VertexBuffer->GetVertexBufferView());
			CommandList->IASetIndexBuffer(&m_IndexBuffer->GetIndexBufferView());

			CommandList->RSSetViewports(1, &m_Viewport);
			CommandList->RSSetScissorRects(1, &m_ScissorRect);

			//bind the heap containing the texture descriptor
			CommandList->SetDescriptorHeaps(1, m_Texture->GetSrvHeap().GetAddressOf());
			CommandList->SetGraphicsRootDescriptorTable(1, m_Texture->GetSrvHeap()->GetGPUDescriptorHandleForHeapStart());
			// bind render target & depth stencil
			CommandList->OMSetRenderTargets(1, &rtv, TRUE, &GetDSVHandle());

			{
				const auto mvp = DirectX::XMMatrixTranspose(
					DirectX::XMMatrixRotationX(1.0f * t + 1.f) *
					DirectX::XMMatrixRotationY(1.2f * t + 2.f) *
					DirectX::XMMatrixRotationZ(1.1f * t + 0.f) *
					m_Projection->GetProjectionView());

				CommandList->SetGraphicsRoot32BitConstants(0, sizeof(mvp) / 4, &mvp, 0);
				CommandList->DrawIndexedInstanced(m_IndexBuffer->GetIndices(), 1, 0, 0, 0);
			}
			// Prepare buffer for presentation
			{
				CommandQueue->Transition(BackBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
			}
			// submit command list
			{
				m_FenceValues[m_CurrentBackBufferIndex] = CommandQueue->ExecuteCommandList(CommandList);
				// present frame
				m_SwapChain->vSync();
				// insert fence to mark command list completion
				CommandQueue->WaitForFenceCompletion(m_FenceValues[m_CurrentBackBufferIndex]);
			}
			t += step;
		}
	}
}