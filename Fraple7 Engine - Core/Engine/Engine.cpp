#include "pch.h"
#include "Engine.h"
#include <d3d12.h>
#include <wrl.h>
#include <Utilities/Common/Common.h>
#include "Graphics/3D Renderer/API/DirectX-12/PipeLineDx.h"
#include "Platform/Abstract/Window.h"

namespace Fraple7
{
	namespace Core
	{
		Engine::Engine(const Window& Window) : m_Window(Window)
		{
			Init();
		}
		Engine::~Engine()
		{
		}
		int Engine::Active()
		{
			PipeLineDx swapChain(m_Window, 2);
			
			while (false)
			{

			}
			return FPL_NOT_IMPLEMENTED;
		}
		int Engine::Init()
		{
			return FPL_NOT_IMPLEMENTED;
		}
	}
}