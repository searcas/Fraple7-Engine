#include <iostream>
#include "Platform/Windows/Window.h"
#include "Engine/Engine.h"
#include "Utilities/Common/Common.h"
#include <thread>


using namespace Fraple7::Core;
int wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
)
{
	Fraple7::Core::WinWindow window(1920, 1080, std::string("Fraple7 Test"));
	Fraple7::Core::Engine engine(window);
	
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
		MessageBox(nullptr, L"No details available", L"Unknown Excaption", MB_OK | MB_ICONEXCLAMATION);
	}

	return 0;
}