#include "pch.h"
#include "directx/d3dx12.h"
#include "Engine.h"
#include "Graphics/3D Renderer/Renderer/Renderer.h"
#include "Studio/Platform/Windows/Window.h"

namespace Fraple7
{
	namespace Core
	{
		Engine::Engine(std::shared_ptr<Studio::Window> Window) 
		{
			m_Window = std::dynamic_pointer_cast<Studio::WinWindow>(Window);
			m_Renderer = std::make_unique<Renderer>(Window);
		}
		Engine::~Engine()
		{
			
		}
		int Engine::Active()
		{
			while (m_Window->Run())
			{
				m_Renderer->Render();
			}
			return FPL_SUCCESS;
		}
		int Engine::Init()
		{
			return FPL_NOT_IMPLEMENTED;
		}
	}
}