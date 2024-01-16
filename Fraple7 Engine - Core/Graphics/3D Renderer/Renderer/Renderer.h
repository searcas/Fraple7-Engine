#pragma once
#include "Graphics/3D Renderer/API/DirectX-12/PipeLineDx.h"
#include "Graphics/3D Renderer/API/DirectX-12/VertexBuffer.h"
#include "Graphics/3D Renderer/API/DirectX-12/PipelineStateObject.h"
#include "Graphics/3D Renderer/API/DirectX-12/IndexBuffer.h"
#include "Graphics/Texture/Texture.h"
#include "Projection.h"
#include "Studio/Platform/Windows/Window.h"

namespace Fraple7
{
	namespace Core
	{
		class Renderer
		{
		private:
			bool m_FullScreen = false;
		public:
			Renderer(Window& window);
			~Renderer() = default;
			void Render();
			void Update();
		private:
			PipeLineDx m_PipeLine;
			VertexBuffer m_VertexBuffer;
			uint32_t m_CurrentBackBufferIndex = 0;
			CD3DX12_RECT m_ScissorRect;
			CD3DX12_VIEWPORT m_Viewport;
			PSO m_PSO;
			Projection m_Projection;
			IndexBuffer m_IndexBuffer;
			Texture m_Texture;
			WinWindow& m_Window;
			std::vector<uint64_t> m_FenceValues;
		};

	}
}