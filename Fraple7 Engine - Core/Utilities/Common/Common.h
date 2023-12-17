#pragma once
#include "Utilities/ErrorHandling/EngineErrorHandler.h"
#include "Utilities/EngineDefines/EngineStatus.h"
#include <wrl.h>
#include "Utilities/Logs/Logs.h"
namespace Fraple7
{
	namespace Core
	{
		extern StatusCode statusCode;
		using Microsoft::WRL::ComPtr;
#define FPL_LOG(error, message,lvl) Logs::Log(__FILE__,__FUNCTION__,__LINE__, error, message, lvl)
	}
}