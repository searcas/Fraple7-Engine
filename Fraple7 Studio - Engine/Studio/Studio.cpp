#include <iostream>
#include "Platform/Windows/Window.h"
#include "Engine/Engine.h"

#include <Windows.h>

int main(void)
{
	int deltaTime = 0;
	
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

	return 0;
}