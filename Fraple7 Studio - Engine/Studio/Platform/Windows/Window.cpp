#include "pch.h"
#include "Window.h"
#include "Utilities/Common/Common.h"
#ifdef WINDOWS

namespace Fraple7
{
	namespace Core
	{
		static LRESULT HandleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lparam) noexcept
		{

			switch (msg)
			{
				case WM_CLOSE:
				{
					PostQuitMessage(0);
					return 0;
				}
				case WM_KILLFOCUS:
				{
					break;
				}
				case WM_ACTIVATE:
					//confine/free cursor on window to foreground/background if cursor disabled

					break;
					/********* KEYBOARD *********/
				case WM_KEYDOWN:
					// syskey commands need to be handled to track ALT key (VK_MENU) and F10
				case WM_SYSKEYDOWN:
				{

					break;
				}
				case WM_KEYUP:
				case WM_SYSKEYUP:
				{

					break;
				}
				case WM_CHAR:

					break;
					//############### MOUSE MESSAGES ########################################
				case WM_MOUSEMOVE:
				{
					const POINTS pt = MAKEPOINTS(lparam);

					break;
				}
				case WM_LBUTTONDOWN:
				{
					SetForegroundWindow(hwnd);

					break;
				}
				case WM_RBUTTONDOWN:
				{
					//stifle this mouse message if imgui want to capture

					break;
				}
				case WM_LBUTTONUP:
				{
					//stifle this mouse message if imgui want to capture

					break;
				}
				case WM_RBUTTONUP:
				{
					//stifle this mouse message if imgui want to capture
					break;
				}
				case WM_MOUSEWHEEL:
				{
					const POINTS pt = MAKEPOINTS(lparam);
					const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
					break;
				}
				// ########### RAW MOUSE MESSAGES ######################//
				case WM_INPUT:
				{

					break;
				}
			}
			return DefWindowProc(hwnd, msg, wParam, lparam);
		}
		static LRESULT HandleMsgThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			//retrieve ptr to window isntance
			Window* const pwnd = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			//forward messsage to window instance handler
			return HandleMsg(hwnd, msg, wParam, lParam);
		}
		LRESULT HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			// use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
			if (msg == WM_NCCREATE)
			{
				// extract ptr to window class from creation data
				const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
				Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
				// set WinAPI-managed user data to store ptr to window instance
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
				// set message proc to normal (non-setup) handler now that setup is finished
				SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&HandleMsgThunk));
				//forward message to window instance handler
				return HandleMsg(hwnd, msg, wParam, lParam);
			}
			//if we get a message before the WM_NCCREATE message, handle with default handler

			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		WinWindow::WinWindow(uint32_t width, uint32_t height, std::string&& name)
		{
			HRESULT result;
			// Register the window class.
			const wchar_t CLASS_NAME[] = L"Sample Window Class";

			WNDCLASS wc = { };

			wc.lpfnWndProc = HandleMsgSetup;
			wc.hInstance = NULL;
			wc.lpszClassName = CLASS_NAME;

			RegisterClass(&wc);

			// Create the window.

				m_Hwnd = CreateWindowEx(
				0,								// Optional window styles.
				CLASS_NAME,                     // Window class
				L"Fraple7 Studio",				// Window text
				WS_OVERLAPPEDWINDOW,            // Window style

				// Size and position
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

				NULL,       // Parent window    
				NULL,       // Menu
				NULL,  // Instance handle
				NULL        // Additional application data
			);

			if (m_Hwnd == NULL)
			{
				(result = GetLastError()) >> statusCode;
			}
			else
				ShowWindow(m_Hwnd, SW_SHOW);
		}

		WinWindow::~WinWindow()
		{
		}

		void WinWindow::Initialize()
		{
		}
		HWND WinWindow::GetHandle()
		{
			return m_Hwnd;
		}

		uint32_t WinWindow::Run() const
		{
			// Run the message loop.

			MSG msg = { };
			msg.hwnd = m_Hwnd;
			while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE) > 0)
			{
				if (msg.message == WM_QUIT )
				{
					// return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
					return msg.wParam;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			return 1;
		}
	}
}

#endif // WINDOWS
