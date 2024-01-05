#pragma once
#include "Graphics/3D Renderer/API/DirectX-12/PipeLineDx.h"
#include "Graphics/3D Renderer/API/DirectX-12/Fence.h"
#include "Graphics/3D Renderer/API/DirectX-12/VertexBuffer.h"
#include "Graphics/3D Renderer/API/DirectX-12/PipelineStateObject.h"
#include "Graphics/3D Renderer/API/DirectX-12/IndexBuffer.h"
#include "Graphics/Texture/Texture.h"
#include "Projection.h"

namespace Fraple7
{
	namespace Core
	{
		class Renderer
		{
		public:
			//This should be in some Event class ...
			void Resize(bool);
			void SetFullScreen(bool fullScreen);
		private:
			bool m_FullScreen = false;
		public:
			Renderer(Window& window);
			~Renderer() = default;
			void Render();
			void Update();
		private:
			Renderer& GetInstance() { return *this; }
		private:
			PipeLineDx m_PipeLine;
			VertexBuffer m_VertexBuffer;
			uint32_t m_BufferCount = 0;
			uint32_t m_CurrentBackBufferIndex = 0;
			CD3DX12_RECT m_ScissorRect;
			CD3DX12_VIEWPORT m_Viewport;
			FenceDx m_Fence;
			std::vector<uint64_t> m_FenceValues;
			PSO m_PSO;
			Projection m_Projection;
			IndexBuffer m_IndexBuffer;
			Texture m_Texture;
			Window& m_Window;
		};

	}
}