#pragma once
#include "Graphics/3D Renderer/Renderer/PipeLineDx.h"
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
			std::unique_ptr<PipeLineDx> m_PipeLine;
			WinWindow& m_Window;
		};

	}
}