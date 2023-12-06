#include <iostream>
#include "Platform/Windows/Window.h"
#include "Engine/Engine.h"

#include <Windows.h>

void OnSize(HWND hwnd, UINT flag, int width, int height)
{
	// Handle resizing
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		int width = LOWORD(lParam);  // Macro to get the low-order word.
		int height = HIWORD(lParam); // Macro to get the high-order word.

		// Respond to the message:
		OnSize(hwnd, (UINT)wParam, width, height);
	}
	break;
	}
	return 0;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	//Fraple7::Core::WinWindow window(1920, 1080, std::string("Fraple7 Test"));

    // Register the window class.
    const char CLASS_NAME[] = "Sample Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        "Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, SW_SHOW);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


	//Fraple7::Core::Engine engine(window);
	try
	{
		//engine.Active();

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