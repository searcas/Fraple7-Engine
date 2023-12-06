#pragma once
#include "Graphics/3D Renderer/API/DirectX-12/PipeLineDx.h"
#include "Graphics/3D Renderer/API/DirectX-12/Fence.h"
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
			uint32_t m_BufferCount;
			uint32_t m_CurrentBackBufferIndex;
			FenceDx m_Fence;
		};

	}
}