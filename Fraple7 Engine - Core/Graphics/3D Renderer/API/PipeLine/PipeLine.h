#pragma once

namespace Fraple7
{
	class PipeLine
	{
		virtual uint32_t Create() = 0;
		virtual uint32_t Destroy() = 0;
		virtual uint32_t RenderTargetView() = 0;
		virtual uint32_t Commands() = 0;
	};
}