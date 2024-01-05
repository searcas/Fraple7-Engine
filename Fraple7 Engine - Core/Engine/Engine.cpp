#include "pch.h"
#include "Engine.h"
#include "directx/d3dx12.h"
#include <wrl.h>
#include <Utilities/Common/Common.h>
#include "Graphics/3D Renderer/Renderer/Renderer.h"
#include "Studio/Platform/Abstract/Window.h"

namespace Fraple7
{
	namespace Core
	{
		Engine::Engine(Window& Window) : m_Window(Window)
		{
			Init();
		}
		Engine::~Engine()
		{
		}
		int Engine::Active()
		{
			Renderer renderer(m_Window);

			while (m_Window.Run())
			{
				renderer.Render();
			}
			return FPL_SUCCESS;
		}
		int Engine::Init()
		{
			return FPL_NOT_IMPLEMENTED;
		}
	}
}