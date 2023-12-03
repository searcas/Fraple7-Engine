#pragma once
#include "Utilities/ErrorHandling/EngineErrorHandler.h"
#include "Utilities/EngineDefines/EngineStatus.h"

namespace Fraple7
{
	namespace Core
	{
		extern StatusCode statusCode;
	}
}
#define FAILED(x) (x != 0) ? true : false