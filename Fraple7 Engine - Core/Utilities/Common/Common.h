#pragma once
#include "Utilities/ErrorHandling/EngineErrorHandler.h"
#include "Utilities/EngineDefines/EngineStatus.h"
#include <wrl.h>
#include "Utilities/Logging/Logs.h"
namespace Fraple7
{
	namespace Core
	{
#undef max
#undef min
		extern StatusCode statusCode;
		using Microsoft::WRL::ComPtr;
#define FPL_LOG(error, message,lvl) Logs::Log(__FILE__,__FUNCTION__,__LINE__, error, message, lvl)
#define FPL_CHECK_STATUS_OK(x) if(x == 0 ){ FPL_LOG("Error occured.", "GetLastError", Logs::LogLevel::LOGICAL_ERROR);\
		HRESULT result = GetLastError();\
		result >> statusCode;}

	}
}