#pragma once
#include "Graphics/3D Renderer/Renderer/PipeLineDx.h"

namespace Fraple7
{
	namespace Core
	{
		class Renderer
		{
		private:
			bool m_FullScreen = false;
		public:
			Renderer(std::shared_ptr<Studio::Window> window);
			~Renderer();
			void Render();
			void Update();
		private:
			std::shared_ptr<PipeLineDx> m_PipeLine;
			std::shared_ptr<Studio::WinWindow> m_Window;
		};

	}
}