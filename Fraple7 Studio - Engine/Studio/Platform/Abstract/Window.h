#pragma once
#include <string>

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
		public:
			const std::string& GetName()	 const { return m_Name; };
			const uint32_t	   GetWidth()	 const { return m_Width; };
			const uint32_t	   GetHeight()	 const { return m_Height; };
		public:
			void SetName(std::string&& setName)  { m_Name = std::move(setName);  };
			void SetWidth(uint32_t width) { m_Width = width; }
			void SetHeight(uint32_t height) { m_Height = height; }
			virtual bool Running() const = 0;
		protected:
			std::string m_Name = "Fraple7 Engine";
			uint32_t m_Width = 1920;
			uint32_t m_Height = 1080;
		};
	}
}
