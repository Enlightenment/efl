// evas_software_win32.c : Defines the entry point for the application.
//

#include "stdafx.h"
#include "evas_software_win32.h"
#include <commctrl.h>

#include "Evas.h"

#include "evas_test_main.h"

#include "Evas_Engine_Software_win32_gdi.h"


#define MAX_LOADSTRING 100


// Global Variables:
HINSTANCE			hInst;			// The current instance
HWND				hwndCB;			// The command bar handle

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass	(HINSTANCE, LPTSTR);
BOOL				InitInstance	(HINSTANCE, int);
LRESULT CALLBACK	WndProc			(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About			(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;



	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_EVAS_SOFTWARE_WIN32);



	setup();
	orig_start_time = start_time = get_time();

	// Main message loop:
	for (;;)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		if(msg.message == WM_QUIT)
			return msg.wParam;

		loop();
		evas_render(evas);
	}

	return msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    It is important to call this function so that the application
//    will get 'well formed' small icons associated with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS	wc;

    wc.style			= CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc		= (WNDPROC) WndProc;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= 0;
    wc.hInstance		= hInstance;
    wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EVAS_SOFTWARE_WIN32));
    wc.hCursor			= 0;
    wc.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName		= 0;
    wc.lpszClassName	= szWindowClass;

	return RegisterClass(&wc);
}

//
//  FUNCTION: InitInstance(HANDLE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
//  COMMENTS:
//
//    In this function, we save the instance handle in a global variable and
//    create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND	hWnd;
	TCHAR	szTitle[MAX_LOADSTRING];			// The title bar text
	TCHAR	szWindowClass[MAX_LOADSTRING];		// The window class name

	hInst = hInstance;		// Store instance handle in our global variable
	// Initialize global strings
	LoadString(hInstance, IDC_EVAS_SOFTWARE_WIN32, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance, szWindowClass);

	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE|WS_CAPTION|WS_SYSMENU,
		0, 0, (int)win_w, (int)win_h+24, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
//	if (hwndCB)
//		CommandBar_Show(hwndCB, TRUE);


    evas = evas_new();
    evas_output_method_set(evas, evas_render_method_lookup("software_win32_gdi"));
    evas_output_size_set(evas, win_w, win_h);
    evas_output_viewport_set(evas, 0, 0, win_w, win_h);

   {
		Evas_Engine_Info_Software_Win32_GDI *einfo;


		einfo = (Evas_Engine_Info_Software_Win32_GDI *)evas_engine_info_get(evas);

		einfo->info.hwnd = hWnd;
		einfo->info.rotation = 0;


			{
				HDC hdc;
				int c;


				hdc = GetDC(hWnd);
				c = GetDeviceCaps(hdc,BITSPIXEL);

				einfo->info.depth = c;


				c = GetDeviceCaps(hdc,RASTERCAPS);


				ReleaseDC(hWnd,hdc);

			}

		evas_engine_info_set(evas, (Evas_Engine_Info *)einfo);
   }






	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	TCHAR szHello[MAX_LOADSTRING];
	RECT rect;


	switch (message)
	{
		case WM_COMMAND:
			wmId    = LOWORD(wParam);
			wmEvent = HIWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
				case IDM_HELP_ABOUT:
				   DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
				   break;
				case IDM_FILE_EXIT:
				   DestroyWindow(hWnd);
				   break;
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_CREATE:
//			hwndCB = CommandBar_Create(hInst, hWnd, 1);
//			CommandBar_InsertMenubar(hwndCB, hInst, IDM_MENU, 0);
//			CommandBar_AddAdornments(hwndCB, 0, 0);
			break;

		case WM_LBUTTONDOWN:
		  evas_event_feed_mouse_move(evas, LOWORD(lParam), HIWORD(lParam));
		  evas_event_feed_mouse_down(evas, 1);
			break;


		case WM_LBUTTONUP:
		  evas_event_feed_mouse_move(evas, LOWORD(lParam), HIWORD(lParam));
		  evas_event_feed_mouse_up(evas, 1);
			break;

		case WM_MOVE:
		  evas_event_feed_mouse_move(evas, LOWORD(lParam), HIWORD(lParam));
			break;

		case WM_SIZE:
			evas_output_size_set(evas, LOWORD(lParam), HIWORD(lParam));
			break;

/*
		case WM_ERASEBKGND:

       // Get window coordinates, and normalize.
	       GetWindowRect(hWnd, &rect);
		    rect.right = rect.right - rect.left;  // Get width.
			 rect.bottom = rect.bottom - rect.top; // Get height.
			 rect.left = rect.top = 0;

 		    evas_damage_rectangle_add(evas,
					    rect.left,
					    rect.top,
					    rect.right,
					    rect.bottom);


			return 1;
			break;
*/

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
		    rect.right = ps.rcPaint.right - ps.rcPaint.left;  // Get width.
			 rect.bottom = ps.rcPaint.bottom - ps.rcPaint.top; // Get height.
			 rect.left = ps.rcPaint.left;
			 rect.top = ps.rcPaint.top;
			 ps.fErase = TRUE;

 		    evas_damage_rectangle_add(evas,
					    rect.left,
					    rect.top,
					    rect.right,
					    rect.bottom);
			EndPaint(hWnd, &ps);
			break;

		case WM_DESTROY:
			CommandBar_Destroy(hwndCB);
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

// Mesage handler for the About box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rt, rt1;
	int DlgWidth, DlgHeight;	// dialog width and height in pixel units
	int NewPosX, NewPosY;

	switch (message)
	{
		case WM_INITDIALOG:
			// trying to center the About dialog
			if (GetWindowRect(hDlg, &rt1)) {
				GetClientRect(GetParent(hDlg), &rt);
				DlgWidth	= rt1.right - rt1.left;
				DlgHeight	= rt1.bottom - rt1.top ;
				NewPosX		= (rt.right - rt.left - DlgWidth)/2;
				NewPosY		= (rt.bottom - rt.top - DlgHeight)/2;

				// if the About box is larger than the physical screen
				if (NewPosX < 0) NewPosX = 0;
				if (NewPosY < 0) NewPosY = 0;
				SetWindowPos(hDlg, 0, NewPosX, NewPosY,
					0, 0, SWP_NOZORDER | SWP_NOSIZE);
			}
			return TRUE;

		case WM_COMMAND:
			if ((LOWORD(wParam) == IDOK) || (LOWORD(wParam) == IDCANCEL))
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}
