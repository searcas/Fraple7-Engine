#include "pch.h"
#include "Window.h"
#include "Utilities/Common/Common.h"

#ifdef WINDOWS

namespace Fraple7
{
	namespace Core
	{
		void WinWindow::Resize()
		{
		
			m_Width = (std::max(1u, m_Width));
			m_Width = (std::max(1u, m_Height));
			if (m_Device.get() == nullptr)
				return;

			m_CommandQueue->SignalAndWait();
			const auto& SwapChainPtr = m_SwapChain->GetSwapChain();
			for (size_t i = 0; i < m_SwapChain->GetBufferCount(); i++)
			{
				m_SwapChain->GetBackBuffer()[i].Reset();
			}
			DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
			SwapChainPtr->GetDesc(&swapChainDesc) >> statusCode;
			SwapChainPtr->ResizeBuffers(m_SwapChain->GetBufferCount(), m_Width, m_Height,
										swapChainDesc.BufferDesc.Format, swapChainDesc.Flags) >> statusCode;

			
			m_SwapChain->RenderTargetView();
		}
		LRESULT CALLBACK WinWindow::HandleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lparam) noexcept
		{
			bool alt = (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
			switch (msg)
			{
				case WM_SIZE:
				{
					RECT currentSize = {};
					::GetClientRect(hwnd, &currentSize);
					int width = currentSize.right - currentSize.left;
					int height = currentSize.bottom - currentSize.top;
					if (m_Width != width || m_Height != height)
					{
						m_Width = width;
						m_Height = height;
						Resize();
					}
					break;
				}
				case WM_CLOSE:
				{
					PostQuitMessage(0);
					break;
				}
				case WM_DESTROY:
				{
					PostQuitMessage(0);
					break;
				}
				case WM_KILLFOCUS:
				{
					break;
				}
				case WM_ACTIVATE:
					//confine/free cursor on window to foreground/background if cursor disabled

					break;
				/********* KEYBOARD *********/
				// syskey commands need to be handled to track ALT key (VK_MENU) and F10
				case WM_SYSKEYDOWN:
				case WM_KEYDOWN:
				{
					switch (wParam)
					{
					case 'G':
						m_vSync = !m_vSync;
						m_SwapChain->SetVSync(m_vSync);
						OutputDebugString("vSync Enabled");
						break;
						if (alt)
						{
					case VK_RETURN:
					case VK_F11:
						SetFullScreen();
						break;
					case VK_F4:
						PostQuitMessage(0);
						break;
						}
						break;
					}
					break;
				}
			/*	case WM_SYSCHAR:
					break;*/
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
				default:
					return DefWindowProc(hwnd, msg, wParam, lparam);
			}
			return DefWindowProc(hwnd, msg, wParam, lparam);
		}
		LRESULT CALLBACK WinWindow::HandleMsgThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			//retrieve ptr to window isntance
			WinWindow* const pwnd = reinterpret_cast<WinWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			//forward messsage to window instance handler
			return pwnd->HandleMsg(hwnd, msg, wParam, lParam);
		}
		LRESULT CALLBACK WinWindow::HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			// use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
			if (msg == WM_NCCREATE)
			{
				// extract ptr to window class from creation data
				const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
				WinWindow* const pWnd = static_cast<WinWindow*>(pCreate->lpCreateParams);
				// set WinAPI-managed user data to store ptr to window instance
				SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
				// set message proc to normal (non-setup) handler now that setup is finished
				SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WinWindow::HandleMsgThunk));
				//forward message to window instance handler
				return pWnd->HandleMsg(hwnd, msg, wParam, lParam);
			}
			//if we get a message before the WM_NCCREATE message, handle with default handler

			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		WinWindow::WinWindow(uint32_t width, uint32_t height, std::string&& name)
		{
			HRESULT result;
			// Register the window class.
			const char CLASS_NAME[] = "Fraple7 Studio";

			m_WC.lpfnWndProc = &HandleMsgSetup;
			m_WC.hInstance = NULL;
			m_WC.lpszClassName = CLASS_NAME;

			RegisterClass(&m_WC);

			m_wRect.left = 100;
			m_wRect.right = width + m_wRect.left;
			m_wRect.top = 100;
			m_wRect.bottom = height + m_wRect.top;
			FPL_CHECK_STATUS_OK(&m_wRect, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);
		
			// Create the window.
			m_hWnd = CreateWindowEx(
				0,								// Optional window styles.
				CLASS_NAME,                     // Window class
				"Fraple7 Studio",				// Window text
				WS_OVERLAPPEDWINDOW,            // Window style

				// Size and position
				CW_USEDEFAULT, CW_USEDEFAULT, width, height,

				NULL,       // Parent window    
				NULL,       // Menu
				NULL,		// Instance handle
				this        // Additional application data
			);

			FPL_CHECK_STATUS_OK(m_hWnd)
			ShowWindow(m_hWnd, SW_SHOW);
		}

		WinWindow::~WinWindow()
		{
		}

		void WinWindow::Initialize()
		{
		}

		void WinWindow::SetFullScreen()
		{
			m_FullScreen = !m_FullScreen;
			if (m_FullScreen)
			{
				m_FullScreen = true;
				FPL_CHECK_STATUS_OK(GetWindowRect(m_hWnd, &m_wRect));
				UINT windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
				FPL_CHECK_STATUS_OK(::SetWindowLongW(m_hWnd, GWL_STYLE, windowStyle));
				// Query the name of the nearest display device for the window.
				// This is required to set the fullscreen dimensions of the window
				// when using a multi-monitor setup.
				HMONITOR hMonitor = ::MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
				MONITORINFOEX monitorInfo = {};
				monitorInfo.cbSize = sizeof(MONITORINFOEX);
				FPL_CHECK_STATUS_OK(GetMonitorInfo(hMonitor, &monitorInfo));

				FPL_CHECK_STATUS_OK(::SetWindowPos(m_hWnd, HWND_TOP,
					monitorInfo.rcMonitor.left,
					monitorInfo.rcMonitor.top,
					monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
					monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOACTIVATE));

				FPL_CHECK_STATUS_OK(::ShowWindow(m_hWnd, SW_MAXIMIZE));
			}
			else
			{
				// Restore all the window decorators.
				::SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

				::SetWindowPos(m_hWnd, HWND_NOTOPMOST,
					m_wRect.left,
					m_wRect.top,
					m_wRect.right - m_wRect.left,
					m_wRect.bottom - m_wRect.top,
					SWP_FRAMECHANGED | SWP_NOACTIVATE);

				::ShowWindow(m_hWnd, SW_NORMAL);
			}
		}


		uint32_t WinWindow::Run() const
		{
			// Run the message loop.

			MSG msg = { };
			msg.hwnd = m_hWnd;
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
