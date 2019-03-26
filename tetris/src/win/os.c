#include <windows.h>
#include <wingdi.h>
#include <string.h>
#include "game.h"
#include "os.h"
#include "render.h"

const char* WINCLASSNAME = "TetrisClass";
const char* WINDOWNAME = "Tetris";

HINSTANCE HInst;
HWND Hwnd;
extern unsigned int* Scene;
BITMAPINFO* BI;
HBITMAP DIBSection;
LARGE_INTEGER Frequency;
LARGE_INTEGER PerformanceCount;

LRESULT CALLBACK wProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
  		case WM_CHAR:
			switch (wParam) {
				case 's':
				case 'S':
					GM_SetKey (KEY_S);
					break;
				case 'p':
				case 'P':
					GM_SetKey (KEY_P);
					break;
				default:
					GM_SetKey (KEY_NONE);
			}

		case WM_KEYDOWN:
			switch (wParam) {
				case VK_LEFT:
					GM_SetKey (KEY_LEFT);
					break;
				case VK_RIGHT:
					GM_SetKey (KEY_RIGHT);
					break;
				case VK_UP:
					GM_SetKey (KEY_UP);
					break;
				case VK_DOWN:
					GM_SetKey (KEY_DOWN);
					break;
				case VK_SPACE:
					GM_SetKey (KEY_SPACE);
					break;
				default:
					GM_SetKey (KEY_NONE);
			}

			return 0;

		case WM_DESTROY:
			PostQuitMessage (0);
			return 0;
	}

	return DefWindowProc (hwnd, uMsg, wParam, lParam);
}

long OS_GetTick () {
	if (!QueryPerformanceCounter (&PerformanceCount)) {
		OS_Exit ("OS_GetTick", "QueryPerformanceCounter");
	}

	// Convert to microseconds.
	PerformanceCount.QuadPart *= 1000000;
	PerformanceCount.QuadPart /= Frequency.QuadPart;

	return (long) PerformanceCount.QuadPart;
}

void OS_Sleep (long milisecs) {
	Sleep (milisecs);
}

#define BUFFER_SIZE 1024
void OS_Exit (char* function, char* failfunction) {
	LPSTR buffer[BUFFER_SIZE];

	FormatMessage (
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError (),
		MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
		buffer,
		BUFFER_SIZE,
		NULL);

	wsprintf ("%s(): %s() failed. '%s'", function, failfunction, buffer);
	exit (EXIT_FAILURE);
}

void OS_Init (HINSTANCE hInst) {
	HDC hdc;
	WNDCLASS wc;
	RECT r;
	DWORD wStyle;

	HInst = hInst;

	wc.style = 0;
	wc.lpfnWndProc = (void*) &wProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = HInst;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_NO);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WINCLASSNAME;

	if (!QueryPerformanceFrequency (&Frequency)) {
		OS_Exit ("OS_Init", "QueryPerformanceFrequency");
	}

	if (!RegisterClass (&wc)) {
		OS_Exit ("OS_Init", "RegisterClass");
	}

	wStyle = WS_CAPTION | WS_SYSMENU | WS_VISIBLE;
	r.left = 0;
	r.top = 0;
	r.right = WINDOW_WIDTH;
	r.bottom = WINDOW_HEIGHT;

	AdjustWindowRect (&r, wStyle, FALSE);

	if ((Hwnd = CreateWindow (WINCLASSNAME, WINDOWNAME, wStyle, CW_USEDEFAULT, CW_USEDEFAULT, r.right-r.left, r.bottom-r.top, NULL,
				NULL, HInst, NULL)) == NULL) {
		OS_Exit ("OS_Init", "CreateWindow");
	}

	UpdateWindow (Hwnd);
	ShowWindow (Hwnd, SW_SHOW);

	if ((hdc = GetDC (Hwnd)) == NULL) {
		OS_Exit ("OS_Init", "GetDC");
	}

	if ((BI = (BITMAPINFO*) malloc (sizeof (BITMAPINFO))) == NULL) {
		OS_Exit ("OS_Init", "malloc");
	}

	BI->bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
	BI->bmiHeader.biWidth = WINDOW_WIDTH;
	BI->bmiHeader.biHeight = WINDOW_HEIGHT;
	BI->bmiHeader.biPlanes = 1;
	BI->bmiHeader.biBitCount = BYTES_PER_PIXEL * 8;
	BI->bmiHeader.biCompression = BI_RGB;
	BI->bmiHeader.biSizeImage = 0;
	BI->bmiHeader.biXPelsPerMeter = 0;
	BI->bmiHeader.biYPelsPerMeter = 0;
	BI->bmiHeader.biClrUsed = 0;
	BI->bmiHeader.biClrImportant = 0;
	BI->bmiColors[0] = (RGBQUAD) {0,0,0,0};

	if ((DIBSection = CreateDIBSection (hdc, BI, DIB_RGB_COLORS, (void*) &Scene, NULL, 0)) == NULL) {
		OS_Exit ("OS_Init", "CreateDIBSection");
	}

	memset (Scene, 0, WINDOW_WIDTH * WINDOW_HEIGHT * BYTES_PER_PIXEL);
	ReleaseDC (Hwnd, hdc);
}

void OS_Render() {
	HDC hdc, chdc;
	HBITMAP hbmp;

	memset (Scene, 0, WINDOW_WIDTH * WINDOW_HEIGHT * BYTES_PER_PIXEL);

	R_Render ();

	if ((hdc = GetDC (Hwnd)) == NULL) {
		OS_Exit ("OS_Render", "GetDC");
	}

	if ((chdc = CreateCompatibleDC (hdc)) == NULL) {
		OS_Exit ("OS_Render", "CreateCompatibleDC");
	}

	if ((hbmp = SelectObject (chdc, DIBSection)) == NULL) {
		OS_Exit ("OS_Render", "SelectObject");
	}

	if (!BitBlt (hdc, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, chdc, 0, 0, SRCCOPY)) {
		OS_Exit ("OS_Render", "BitBlt");
	}

	ReleaseDC (Hwnd, hdc);
	SelectObject (chdc, hbmp);
	DeleteDC (chdc);
}

void OS_ProcessEvents () {
	MSG msg;
	//BOOL exit;

	//exit = FALSE;
	//while (!exit) {
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
			//if (msg.message == WM_QUIT)
			//	exit = TRUE;
		} else {
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}

		//GM_Update ();

		//if (!M_RenderScene ())
		//	return -1;
	//}

	//return msg.wParam;
}

void OS_Terminate() {
	UnregisterClass (WINCLASSNAME, HInst);

	if (DIBSection) {
		DeleteObject (DIBSection);
	}
}
