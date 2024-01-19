#include "pch.h"
#include "Engine/Engine.h"
#include "Utilities/Common/Common.h"
#include "Platform/Windows/Window.h"


using namespace Fraple7::Core;
int wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	std::shared_ptr<Fraple7::Studio::WinWindow> window = std::make_shared<Fraple7::Studio::WinWindow>(1920, 1080, std::string("Fraple7 Test"));
	std::shared_ptr<Fraple7::Studio::Window> win = std::static_pointer_cast<Fraple7::Studio::Window>(window);
	Engine engine(win);
	try
	{
		 engine.Active(); 
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "error", MB_ICONERROR | MB_SETFOREGROUND);
	}
	catch (...) 
	{
		MessageBox(nullptr, "No details available", "Unknown Excaption", MB_OK | MB_ICONEXCLAMATION);
	}

	return 0;
}