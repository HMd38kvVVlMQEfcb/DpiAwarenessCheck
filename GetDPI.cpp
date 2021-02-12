// GetDPI.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "GetDPI.h"
#include <strsafe.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GETDPI, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GETDPI));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GETDPI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GETDPI);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


static void TryGetDpiForMonitor(HWND hWnd, LPWSTR ResultBuffer, size_t ResultBufferCch)
{
	HMODULE shcore = LoadLibraryW(L"shcore");
	if (!shcore)
	{
		StringCchPrintfW(ResultBuffer, ResultBufferCch, L"shcore doesn't exist on this system.");
		return;
	}

	auto pfn_GetDpiForMonitor = (HRESULT (WINAPI *)(HMONITOR, DWORD, UINT*, UINT*))GetProcAddress(shcore, "GetDpiForMonitor");
	if (pfn_GetDpiForMonitor == nullptr)
	{
		StringCchPrintfW(ResultBuffer, ResultBufferCch, L"Function \"GetDpiForMonitor\" doesn't exist on this system.");
	}
	else
	{
		HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
		UINT dpi_x = 0;
		UINT dpi_y = 0;
		if (SUCCEEDED(pfn_GetDpiForMonitor(monitor, 0, &dpi_x, &dpi_y)))
		{
			StringCchPrintfW(ResultBuffer, ResultBufferCch, L"GetDpiForMonitor: %u %u.", dpi_x, dpi_y);
		}
		else
		{
			StringCchPrintfW(ResultBuffer, ResultBufferCch, L"GetDpiForMonitor failed.");
		}
	}

	FreeLibrary(shcore);
}


static void TryGetDpiAwarenessLevel(LPWSTR ResultBuffer, size_t ResultBufferCch)
{
	HMODULE user32 = LoadLibraryW(L"user32");
	if (user32 == NULL)
	{
		StringCchPrintfW(ResultBuffer, ResultBufferCch, L"You don't have user32 (wat?)");
		return;
	}

	auto pfn_GetThreadDpiAwarenessContext = (HANDLE (WINAPI *)())GetProcAddress(user32, "GetThreadDpiAwarenessContext");
	if (pfn_GetThreadDpiAwarenessContext == nullptr)
	{
		StringCchPrintfW(ResultBuffer, ResultBufferCch, L"Function \"GetThreadDpiAwarenessContext\" doesn't exist on this system.");
	}
	else
	{
		auto pfn_GetAwarenessFromDpiAwarenessContext = (UINT (WINAPI *)(HANDLE))GetProcAddress(user32, "GetAwarenessFromDpiAwarenessContext");
		if (pfn_GetAwarenessFromDpiAwarenessContext == nullptr)
		{
			StringCchPrintfW(ResultBuffer, ResultBufferCch, L"Function \"GetThreadDpiAwarenessContext\" doesn't exist on this system.");
		}
		else
		{
			HANDLE ThreadDpiAwarenessContext = pfn_GetThreadDpiAwarenessContext();
			UINT DpiAwarenessLevel = pfn_GetAwarenessFromDpiAwarenessContext(ThreadDpiAwarenessContext);
			LPCWSTR LevelText;
			switch (DpiAwarenessLevel)
			{
				case DPI_AWARENESS_UNAWARE:
					LevelText = L"DPI_AWARENESS_UNAWARE";
					break;
				case DPI_AWARENESS_SYSTEM_AWARE:
					LevelText = L"DPI_AWARENESS_SYSTEM_AWARE";
					break;
				case DPI_AWARENESS_PER_MONITOR_AWARE:
					LevelText = L"DPI_AWARENESS_PER_MONITOR_AWARE";
					break;
				default:
					LevelText = L"invalid";
					break;
			}
			StringCchPrintfW(ResultBuffer, ResultBufferCch, L"awareness: %d (%s)", (int)DpiAwarenessLevel, LevelText);
		}
	}

	FreeLibrary(user32);
}


#define IDC_BUTTON_GETDPI_DEVICECAPS 901
#define IDC_BUTTON_GETDPI_GETDPIFORMONITOR 902
#define IDC_BUTTON_GETDPIAWARENESS 903


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
		case WM_CREATE:
		{
			CreateWindowExW(0, L"BUTTON", L"Get DPI (GetDeviceCaps)", WS_VISIBLE | WS_CHILD, 50, 50, 250, 48, hWnd, (HMENU)IDC_BUTTON_GETDPI_DEVICECAPS, hInst, nullptr);
			CreateWindowExW(0, L"BUTTON", L"Get DPI (GetDpiForMonitor)", WS_VISIBLE | WS_CHILD, 50, 110, 250, 48, hWnd, (HMENU)IDC_BUTTON_GETDPI_GETDPIFORMONITOR, hInst, nullptr);
			CreateWindowExW(0, L"BUTTON", L"Get DPI Awareness level", WS_VISIBLE | WS_CHILD, 50, 170, 250, 48, hWnd, (HMENU)IDC_BUTTON_GETDPIAWARENESS, hInst, nullptr);
			break;
		}

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case IDC_BUTTON_GETDPI_DEVICECAPS:
			{
				HDC hdc = GetDC(hWnd);
				int x = GetDeviceCaps(hdc, LOGPIXELSX);
				int y = GetDeviceCaps(hdc, LOGPIXELSY);
				WCHAR buf[100];
				StringCchPrintfW(buf, _countof(buf), L"X: %d -- Y: %d", x, y);
				MessageBoxW(hWnd, buf, L"DPI (GetDeviceCaps)", MB_OK);
				ReleaseDC(hWnd, hdc);
				break;
			}
			case IDC_BUTTON_GETDPI_GETDPIFORMONITOR:
			{
				WCHAR buf[100];
				TryGetDpiForMonitor(hWnd, buf, _countof(buf));
				MessageBoxW(hWnd, buf, L"DPI (GetDpiForMonitor)", MB_OK);
				break;
			}
			case IDC_BUTTON_GETDPIAWARENESS:
			{
				WCHAR buf[100];
				TryGetDpiAwarenessLevel(buf, _countof(buf));
				MessageBoxW(hWnd, buf, L"awareness level", MB_OK);
				break;
			}
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
