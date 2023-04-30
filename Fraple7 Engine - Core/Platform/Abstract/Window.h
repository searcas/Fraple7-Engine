#pragma once
#include "../../pch.h"




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
			const std::string& GetName()	 const { return _mName; };
			const uint32_t	   GetWidth()	 const { return _mWidth; };
			const uint32_t	   GetHeight()	 const { return _mHeight; };
		public:
			void SetName(std::string&& setName)  { _mName = std::move(setName);  };
			void SetWidth(uint32_t width) { _mWidth = width; }
			void SetHeight(uint32_t height) { _mHeight = height; }
		protected:
			std::string _mName = "Fraple7 Engine";
			uint32_t _mWidth = 1920;
			uint32_t _mHeight = 1080;
		};
	}
}
