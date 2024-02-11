#pragma once
#include "Utilities/ErrorHandling/EngineErrorHandler.h"
#include "Utilities/EngineDefines/EngineStatus.h"
#include <wrl.h>
#include "Utilities/Logging/Logs.h"
#include "ForwardDeclarations.h"
#include <assert.h>
#define _KB(x) (x * 1024)
#define _MB(x) (x * 1024 * 1024)

#define _64KB _KB(64)
#define _1MB _MB(1)
#define _2MB _MB(2)
#define _4MB _MB(4)
#define _8MB _MB(8)
#define _16MB _MB(16)
#define _32MB _MB(32)
#define _64MB _MB(64)
#define _128MB _MB(128)
#define _256MB _MB(256)

namespace Fraple7
{
	namespace Core
	{
#undef max
#undef min
		extern StatusCode statusCode;
		using Microsoft::WRL::ComPtr;

#define FPL_LOG(error, message,lvl) ::Logs::Log(__FILE__,__FUNCTION__,__LINE__, error, message, lvl)

#define FPL_CHECK_STATUS_OK(x) \
		if(x == 0) { \
			FPL_LOG("Error occured.", "GetLastError", ::Logs::LogLevel::LOGICAL_ERROR);\
			HRESULT result = GetLastError();\
			result >> ::Fraple7::Core::statusCode;	\
		}
	}
}