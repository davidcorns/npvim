/*
	Author : David Corns

	This demo application using Scintilla was found from internet, and was used for develop a vi-like editor in Windows enviroment

	The main.cpp, DMApp class(DMApp.h, DMApp.cpp), ressource.h are from the origanal demo, and they are mostly likely remain origin.
	The rest are newly added for the vi-like functionility.
	
	The trick used here is called complie-time compitibility. 
	A child class MyApp was inherite from DMApp, and it contains the interface of DMApp.
	Thus, the most changes to the demo code is to replace the DMApp defination to MyApp, and that's it.	
	
	(see line 22 of main.cpp: static MyApp app;)
*/

// A simple demonstration application using Scintilla
#include <windows.h>
#include <resource.h>

#include <constants.h>
#include <MyApp.h>


static MyApp app;


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_CREATE:
		app.wEditor = ::CreateWindow(
		              "Scintilla",
		              "Source",
		              WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_CLIPCHILDREN,
		              0, 0,
		              100, 100,
		              hWnd,
		              0,
		              app.hInstance,
		              0);
		app.InitialiseEditor();
		::ShowWindow(app.wEditor, SW_SHOW);
		::SetFocus(app.wEditor);
		return 0;

	case WM_SIZE:
		if (wParam != 1) {
			RECT rc;
			::GetClientRect(hWnd, &rc);
			::SetWindowPos(app.wEditor, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0);
		}
		return 0;

	case WM_COMMAND:
		app.Command(LOWORD(wParam));
		app.CheckMenus();
		return 0;

	case WM_NOTIFY:
		app.Notify(reinterpret_cast<SCNotification *>(lParam));
		return 0;

	case WM_MENUSELECT:
		app.CheckMenus();
		return 0;

	case WM_CLOSE:
		if (app.SaveIfUnsure() != IDCANCEL) {
			::DestroyWindow(app.wEditor);
			::PostQuitMessage(0);
		}
		return 0;
		
	case WM_CHAR:
		return 0;
	

	default:
		return DefWindowProc(hWnd, iMessage, wParam, lParam);
	}
}

static void RegisterWindowClass() {
	const char resourceName[] = "DMApp";

	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = app.hInstance;
	wndclass.hIcon = 0;
	wndclass.hCursor = NULL;
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = resourceName;
	wndclass.lpszClassName = className;

	if (!::RegisterClass(&wndclass))
		::exit(FALSE);
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpszCmdLine, int nCmdShow) {
	app.hInstance = hInstance;

	HACCEL hAccTable = LoadAccelerators(hInstance, "DMApp");

	//::LoadLibrary("Scintilla.DLL");
	::LoadLibrary("SciLexer.DLL");

	RegisterWindowClass();

	app.wMain = ::CreateWindowEx(
	             WS_EX_CLIENTEDGE,
	             className,
	             "Notepad",
	             WS_CAPTION | WS_SYSMENU | WS_THICKFRAME |
	             WS_MINIMIZEBOX | WS_MAXIMIZEBOX |
	             WS_MAXIMIZE | WS_CLIPCHILDREN,
				 CW_USEDEFAULT, CW_USEDEFAULT, 
				 CW_USEDEFAULT, CW_USEDEFAULT, 
	             NULL,
	             NULL,
	             app.hInstance,
	             0);
	app.SetTitle();
	::ShowWindow(app.wMain, nCmdShow);

	bool going = true;
	MSG msg;
	msg.wParam = 0;
	while (going) {
		going = GetMessage(&msg, NULL, 0, 0);
		if (app.currentDialog && going) {
			if (!IsDialogMessage(app.currentDialog, &msg)) {
				if (TranslateAccelerator(msg.hwnd, hAccTable, &msg) == 0) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		} else if (going) {
			if (TranslateAccelerator(app.wMain, hAccTable, &msg) == 0) {
				app.Process(&msg);
			}
		}
	}

	return msg.wParam;
}
