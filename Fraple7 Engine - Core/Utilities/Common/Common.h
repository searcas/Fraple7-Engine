#pragma once
#include "Utilities/ErrorHandling/EngineErrorHandler.h"
#include "Utilities/EngineDefines/EngineStatus.h"
#include <wrl.h>
namespace Fraple7
{
	namespace Core
	{
		extern StatusCode statusCode;
		using Microsoft::WRL::ComPtr;
	}
}
#define FAILED(x) (x != 0) ? true : false