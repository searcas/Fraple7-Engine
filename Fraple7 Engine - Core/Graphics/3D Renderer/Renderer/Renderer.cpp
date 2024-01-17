#include "pch.h"
#include "Renderer.h"
#include <DirectXMath.h>

namespace Fraple7
{
	namespace Core
	{
		// TODO://
		// Make proper Rendering Technique 
		// as
		// ExecuteIndirect
		// FrameBuffering sample
		// for number of backbuffers - >
		// https ://youtu.be/E3wTajGZOsA?si=HMrmKn0jrgJeZ_VW>
		// https ://jackmin.home.blog/2018/12/14/swapchains-present-and-present-latency/
		// https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12ExecuteIndirect/src/DXSample.cpp
		Renderer::Renderer(Window& window)
			: m_Window((WinWindow&)window)
			
		{
			m_PipeLine = std::make_unique<PipeLineDx>(m_Window);
			m_PipeLine->Init();
		}
	
	
		void Renderer::Render()
		{
			Update();
			m_PipeLine->Render();
		}


		void Renderer::Update()
		{
			static uint64_t frameCounter = 0;
			static double elapsedSeconds = 0.0;
			static std::chrono::high_resolution_clock clock;
			static auto t0 = clock.now();

			frameCounter++;
			auto t1 = clock.now();
			auto deltaTime = t1 - t0;
			t0 = t1;

			//Converting from nanoseconds to seconds 1 x 10^-9
			elapsedSeconds += deltaTime.count() * 1e-9;

			// Print every second only
			if (elapsedSeconds > 1.0)
			{
				char buffer[128];
				auto fps = frameCounter / elapsedSeconds;
				sprintf_s(buffer, 128, "FPS: %f\n", fps);
				OutputDebugString(buffer);

				frameCounter = 0;
				elapsedSeconds = 0.0;
			}
		}

	}
}
