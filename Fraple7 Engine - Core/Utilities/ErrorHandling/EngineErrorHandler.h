#pragma once
#include <source_location>

namespace Fraple7
{
	namespace Core
	{
		class StatusCode {};

		class ErrorHandler
		{
		friend void operator >>(ErrorHandler, StatusCode);
		public:
			ErrorHandler(uint32_t hr, std::source_location = std::source_location::current()) noexcept;
			~ErrorHandler() = default;
		private:
			uint32_t _mHR;
			std::source_location _mLocation;
		};

		void operator >> (ErrorHandler, StatusCode);
	}
}