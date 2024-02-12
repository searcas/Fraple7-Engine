#include "pch.h"
#include "PipeLineDx.h"
#include "Studio/Platform/Windows/Window.h"
#include "Graphics/3D Renderer/API/DirectX-12/SwapChain.h"
#include "Graphics/3D Renderer/API/DirectX-12/Memory/DepthBuffer.h"
#include "Graphics/3D Renderer/API/DirectX-12/Memory/VertexBuffer.h"
#include "Graphics/3D Renderer/API/DirectX-12/Memory/IndexBuffer.h"

#include "Graphics/3D Renderer/API/DirectX-12/CommandList.h"
#include "Graphics/3D Renderer/API/DirectX-12/CommandQueue.h"
#include "Graphics/3D Renderer/API/DirectX-12/CommandMgr.h"
#include "Graphics/3D Renderer/API/DirectX-12/PipelineStateObject.h"
#include "Graphics/3D Renderer/Renderer/View/Projection.h"


namespace Fraple7
{
	namespace Core
	{
		PipeLineDx::PipeLineDx(const std::shared_ptr<Studio::Window>& window) :
			m_Window(std::dynamic_pointer_cast<Studio::WinWindow>(window))
		{
			m_Device = std::make_shared<Device>();

			m_CommandMgr = std::make_shared<CommandMgr>();

			m_DepthBuffer = std::make_shared<DepthBuffer>(m_Window, m_CommandMgr);

			m_SwapChain = std::make_shared<SwapChain>(window, m_Device, 3, m_CommandMgr->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));
			std::wstring name = L"ShibaShitu.png";
			m_Texture = std::make_unique<Texture>(name);
			m_PSO = std::make_unique<PSO>();
			m_VertexBuffer = std::make_unique<VertexBuffer>(L"VertexBuffer");
			m_IndexBuffer = std::make_unique<IndexBuffer>(L"IndexBuffer");
			m_Projection = std::make_unique<Projection>(window);
			m_FenceValues.resize(m_SwapChain->GetBufferCount());

		}
		const CD3DX12_CPU_DESCRIPTOR_HANDLE& PipeLineDx::GetDSVHandle() 
		{ return m_DepthBuffer->GetDSVHandle(); }

		PipeLineDx::~PipeLineDx()
		{
			m_CommandMgr->UnloadAll();
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

			m_VertexBuffer->Create();
			m_DepthBuffer->Create();

			const auto& ComQueue = m_CommandMgr->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
			const auto& ComList = ComQueue->GetCommandList();

			ComQueue->Join(ComList,m_VertexBuffer->GetVertexBuffer(), m_VertexBuffer->GetVertexUploadBuffer());
			// insert fence to detect when upload is complete

			m_VertexBuffer->CreateVertexBufferView();
			m_IndexBuffer->Create();

			ComQueue->Join(ComList,m_IndexBuffer->GetIndexBuffer(), m_IndexBuffer->GetIndexUploadBuffer());

			m_IndexBuffer->CreateIndexBufferView();
			m_Texture->Create();

			ComQueue->Join(ComList, m_Texture->GetTextureBuffer(), m_Texture->GetTextureUploadBuffer(), m_Texture->GetSubData().size(), m_Texture->GetSubData());

			m_Texture->DescriptorHeap();
			// Create descriptor in the heap
			m_Texture->ShaderResourceViewDesc();

			m_PSO->Create();

			// Ensures that scrissor rectangle covers the entire screen 
			// regardless of the size of the screen
			m_ScissorRect = CD3DX12_RECT{ 0,0, LONG_MAX, LONG_MAX };
			// specifices the viewable part of the screen to render to
			m_Viewport = CD3DX12_VIEWPORT{ 0.0f, 0.0f, float(m_Window->GetWidth()), float(m_Window->GetHeight()) };
			
			m_Projection->SetView();

			m_DepthBuffer->InitDescriptorHeap();
			m_DepthBuffer->CreateDepthStencilView();
			m_InitComplete = true;
			
			ComQueue->WaitForFenceCompletion(ComQueue->ExecuteCommandList(ComList));
		}
		float t = 0.0f;
		float step = 0.01f;
		void PipeLineDx::Render()
		{
			++m_FrameNumber;
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

				//FLOAT clearColor[]{ 1.0f, 1.0f, 1.0f, 1.0f };
				CommandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
			}
			CommandList->ClearDepthStencilView(GetDSVHandle(), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
			// Set Pipeline state
			CommandList->SetPipelineState(m_PSO->GetPLS().Get());
			CommandList->SetGraphicsRootSignature(m_PSO->GetRootSig().GetRootSignature().Get());

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
			CommandList->OMSetRenderTargets(1, &rtv, FALSE, &GetDSVHandle());

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
				PresentFrames(m_vSync);
				// insert fence to mark command list completion
				CommandQueue->WaitForFenceCompletion(m_FenceValues[m_CurrentBackBufferIndex]);
			}
			t += step;
		}
		void PipeLineDx::PresentFrames(bool set )
		{
			m_SwapChain->vSync(set);
		}
		void PipeLineDx::Resize()
		{
		
			m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_Window->GetWidth()), static_cast<float>(m_Window->GetHeight()));
			m_SwapChain->ResizeSwapChain();
			m_DepthBuffer->ResizeDepthBuffer();
		}
	}
}