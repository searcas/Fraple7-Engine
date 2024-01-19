#pragma once
#include "Utilities/Common/ForwardDeclarations.h"
namespace Fraple7
{
	namespace Core
	{
		class Renderer;

		class Engine
		{
		public:
			Engine(std::shared_ptr<Studio::Window> Window);
			~Engine();
			int Active();
			int Init();
		private:
			std::shared_ptr<Studio::WinWindow> m_Window;
			std::unique_ptr<Renderer> m_Renderer;
		};

	}
}

