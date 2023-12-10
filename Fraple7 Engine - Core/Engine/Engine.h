#pragma once
namespace Fraple7
{
	namespace Core
	{
		class Engine
		{
		public:
			Engine(const class Window& window);
			~Engine();
			int Active();
			int Init();
		private:
			const class Window& m_Window;
			std::atomic_bool m_Stop = false;
			
		};

	}
}

