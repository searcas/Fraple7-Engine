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
			HWND GetHandle() const  override { return m_hWnd; }
			HWND GetRect() { return m_hWnd; }
			RECT GetWindowPosition() { return m_wRect; }
			void SetFullScreen(bool) override;
			bool Resize(uint32_t width, uint32_t height) override;
		private:
			LRESULT CALLBACK HandleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lparam) noexcept;
			static LRESULT HandleMsgThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
			static LRESULT HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		private:
			HWND m_hWnd;
			WNDCLASS m_WC = { };
			RECT m_wRect; // Position of window
		};
	}
}