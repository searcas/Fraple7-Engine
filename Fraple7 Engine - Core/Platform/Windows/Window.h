#pragma once

#include "../Abstract/Window.h"


namespace Fraple7
{
	namespace Core
	{
		class WinWindow : public Window
		{
		public:
			WinWindow(uint32_t width, uint32_t height, std::string&& name);
			~WinWindow();
			void Initialize() override;


		private:

		};


	}
}