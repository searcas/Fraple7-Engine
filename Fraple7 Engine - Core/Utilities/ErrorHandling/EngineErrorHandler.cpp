
#include "pch.h"
#include "EngineErrorHandler.h"
#include "Utilities/Common/Common.h"
#include <Windows.h>
namespace Fraple7
{
	namespace Core
	{
		StatusCode statusCode;
		std::wstring GetErrorDescription(HRESULT hr)
		{
			wchar_t* descriptionWinalloc = nullptr;
			const auto result = FormatMessageW(
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				reinterpret_cast<LPWSTR>(&descriptionWinalloc), 0, nullptr
			);

			std::wstring description;
			if (!result) {
				//chilog.warn(L"Failed formatting windows error");
			}
			else {
				description = descriptionWinalloc;
				if (LocalFree(descriptionWinalloc)) {
					//chilog.warn(L"Failed freeing memory for windows error formatting");
				}
				if (description.ends_with(L"\r\n")) {
					description.resize(description.size() - 2);
				}
			}
			return description;
		}
		std::string ToNarrow(const std::wstring& wide)
		{
			std::string narrow;
			narrow.resize(wide.size() * 2);
			size_t actual;
			wcstombs_s(&actual, narrow.data(), narrow.size(), wide.c_str(), _TRUNCATE);
			narrow.resize(actual - 1);
			return narrow;
		}
		ErrorHandler::ErrorHandler(uint32_t hr, std::source_location location) noexcept 
			: _mHR(hr), _mLocation(location)
		{

		}
	
		void operator>>(ErrorHandler error, StatusCode)
		{
			if (FAILED(error._mHR))
			{
				// get error description as narrow string with crlf removed
				auto errorString = ToNarrow(GetErrorDescription(error._mHR)) |
					std::ranges::views::transform([](char c) { return c == '\n' ? ' ' : c; }) | 
					std::ranges::views::filter([](char c) { return c != '\r'; });
				
				std::string errorStringOrg;
				std::ranges::copy(errorString, std::back_inserter( errorStringOrg));

				throw std::runtime_error{ std::format("Graphics Error: {}\n   {}({})",
					errorStringOrg,
					error._mLocation.file_name(),
					error._mLocation.line()) 
				};

			}
		}
	}
}