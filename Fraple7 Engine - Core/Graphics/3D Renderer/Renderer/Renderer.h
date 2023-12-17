#pragma once
#include "Graphics/3D Renderer/API/DirectX-12/PipeLineDx.h"
#include "Graphics/3D Renderer/API/DirectX-12/Fence.h"
#include "Graphics/3D Renderer/API/DirectX-12/VertexBuffer.h"
#include "Graphics/3D Renderer/API/DirectX-12/PipelineStateObject.h"

namespace Fraple7
{
	namespace Core
	{
		class Renderer
		{
		public:
			Renderer(const class Window& window);
			~Renderer() = default;
			void Render();
		private:
			PipeLineDx m_PipeLine;
			VertexBuffer m_VertexBuffer;
			uint32_t m_BufferCount;
			uint32_t m_CurrentBackBufferIndex;
			CD3DX12_RECT m_ScissorRect;
			CD3DX12_VIEWPORT m_Viewport;
			FenceDx m_Fence;
			PSO m_PSO;
		};

	}
}