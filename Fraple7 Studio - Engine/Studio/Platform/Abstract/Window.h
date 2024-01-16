#pragma once
#include <string>
#ifdef _WIN64
#include <Windows.h>
#endif
namespace Fraple7
{
	namespace Core
	{
		class Window
		{
		public:
			virtual void Initialize() = 0;
			virtual ~Window() = default;
			Window() = default;
			virtual void SetFullScreen() = 0;
			virtual void SetResize(bool) = 0;
			virtual bool GetResize()const = 0;
			virtual uint32_t Run() const = 0;
			virtual HWND GetHandle() const = 0;

		public:
			const std::string& GetName()	 const { return m_Name; };
			const uint32_t	   GetWidth()	 const { return m_Width; };
			const uint32_t	   GetHeight()	 const { return m_Height; };
			const bool GetIsFullScreen() { return m_FullScreen; }
			const void SetIsFullScreen(bool set) { m_FullScreen = set; }
		public:
			void SetName(std::string&& setName)  { m_Name = std::move(setName);  };
			void SetWidth(uint32_t width) { m_Width = width; }
			void SetHeight(uint32_t height) { m_Height = height; }
		protected:
			std::string m_Name = "Fraple7 Engine";
			uint32_t m_Width = 1920;
			uint32_t m_Height = 1080;
			bool m_FullScreen = false;
		};
	}
}
