#pragma once
#include "../Abstract/Window.h"
#include <Windows.h>

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
			uint32_t Run()const override;
			static HWND GetHandle();
			static void SetInstance(HINSTANCE instance) { m_Hinst = instance; }
		private:
			static inline HWND m_Hwnd;
			inline static HINSTANCE m_Hinst;
		};
	}
}