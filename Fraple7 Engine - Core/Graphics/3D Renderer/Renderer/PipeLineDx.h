#pragma once
#include "directx/d3dx12.h"
#include <wrl.h>
#include <dxgi1_6.h>

#include "Graphics/3D Renderer/API/DirectX-12/Command.h"
#include "Graphics/3D Renderer/API/DirectX-12/DepthBuffer.h"
#include <vector>
#include "Graphics/3D Renderer/API/DirectX-12/Device.h"
#include "Graphics/3D Renderer/API/DirectX-12/SwapChain.h"

#include "Graphics/3D Renderer/API/DirectX-12/VertexBuffer.h"
#include "Graphics/3D Renderer/API/DirectX-12/PipelineStateObject.h"
#include "Graphics/3D Renderer/API/DirectX-12/IndexBuffer.h"
#include "Graphics/Texture/Texture.h"

#include "View/Projection.h"

namespace Fraple7
{
	namespace Core
	{
		class PipeLineDx
		{
		private:
			
		public:
			PipeLineDx(class Window& window);
			PipeLineDx() = default;
			~PipeLineDx();
			const CD3DX12_CPU_DESCRIPTOR_HANDLE& GetDSVHandle() { return m_DepthBuffer->GetDSVHandle(); }
		public:
			std::shared_ptr<Device>& GetDevice();
			std::shared_ptr<SwapChain>& GetSwapChain();
			void CleanCommandQueue();
			void Init();
			void Render();
		private:
			std::shared_ptr<Device>m_Device;
			std::shared_ptr<SwapChain> m_SwapChain;

			WinWindow& m_Window;
			std::shared_ptr<DepthBuffer> m_DepthBuffer;
			bool m_InitComplete = false;
			std::unique_ptr<VertexBuffer> m_VertexBuffer;
			std::unique_ptr<PSO> m_PSO;
			std::unique_ptr<IndexBuffer> m_IndexBuffer;
			std::unique_ptr<Texture> m_Texture;
			CD3DX12_RECT m_ScissorRect;
			CD3DX12_VIEWPORT m_Viewport;
			std::unique_ptr<Projection>m_Projection;
			uint32_t m_CurrentBackBufferIndex = 0;
			std::vector<uint64_t> m_FenceValues;
			std::shared_ptr<CommandMgr> m_CommandMgr;
		};

	}
}