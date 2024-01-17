#pragma once
#include "../Abstract/Window.h"
#include <Windows.h>
#include "Graphics/3D Renderer/API/DirectX-12/Device.h"
#include "Graphics/3D Renderer/API/DirectX-12/SwapChain.h"
#include "Graphics/3D Renderer/API/DirectX-12/DepthBuffer.h"
namespace Fraple7
{
	namespace Core
	{
		class WinWindow : public Window
		{
		public:
			WinWindow(uint32_t width, uint32_t height, std::string&& name);
			WinWindow() = default;
			~WinWindow();
			void Initialize() override;
			uint32_t Run()const override;
			HWND GetHandle() const  override { return m_hWnd; }
			RECT GetWindowPosition() const { return m_wRect; }
			void SetFullScreen() override;
			void SetResize(bool resize)override { m_Resize = resize; }
			bool GetResize() const override { return m_Resize; }
		public:
			void SetSwapChainRef(std::shared_ptr<SwapChain>& ref) { m_SwapChain = ref; }
			void SetDepthBufferRef(std::shared_ptr<DepthBuffer>& ref) { m_DepthBuffer = ref; }
		private:
			LRESULT CALLBACK HandleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lparam) noexcept;
			static LRESULT HandleMsgThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
			static LRESULT HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		private:
			HWND m_hWnd;
			WNDCLASS m_WC = { };
			RECT m_wRect; // Position of window
			bool m_Resize = false;
			bool m_vSync = false;
		private:
			std::shared_ptr<SwapChain>m_SwapChain;
			std::shared_ptr<DepthBuffer> m_DepthBuffer;
		};
	}
}