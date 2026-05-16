///////////////////////////////////////////////////////////////////////////
// 
//  Copyright 2026 by Pavel Chistyakov
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http ://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "draw.h"
#include "slice.h"
#include "win.h"

#define ZOOM 2

#pragma pack(push)
#pragma pack(1)
static struct video_8t {
	BITMAPINFO bmp;
	unsigned char bmp_pallette[256 * 4];
} video_descriptor;
#pragma pack(pop)

static HWND	hwnd;
static point minimum;
static surface video_buffer;

void scale2x(void* void_dst, unsigned dst_slice, const void* void_src, unsigned src_slice, unsigned width, unsigned height);

static struct sys_key_mapping {
	unsigned    key;
	unsigned    id;
} sys_key_mapping_data[] = {
	{VK_CONTROL, Ctrl},
	{VK_MENU, Alt},
	{VK_SHIFT, Shift},
	{VK_LEFT, KeyLeft},
	{VK_RIGHT, KeyRight},
	{VK_UP, KeyUp},
	{VK_DOWN, KeyDown},
	{VK_PRIOR, KeyPageUp},
	{VK_NEXT, KeyPageDown},
	{VK_HOME, KeyHome},
	{VK_END, KeyEnd},
	{VK_BACK, KeyBackspace},
	{VK_DELETE, KeyDelete},
	{VK_RETURN, KeyEnter},
	{VK_ESCAPE, KeyEscape},
	{VK_SPACE, KeySpace},
	{VK_TAB, KeyTab},
	{VK_F1, F1},
	{VK_F2, F2},
	{VK_F3, F3},
	{VK_F4, F4},
	{VK_F5, F5},
	{VK_F6, F6},
	{VK_F7, F7},
	{VK_F8, F8},
	{VK_F9, F9},
	{VK_F10, F10},
	{VK_F11, F11},
	{VK_F12, F12},
	{VK_MULTIPLY, (unsigned)'*'},
	{VK_DIVIDE, (unsigned)'/'},
	{VK_ADD, (unsigned)'+'},
	{VK_SUBTRACT, (unsigned)'-'},
	{VK_OEM_COMMA, (unsigned)','},
	{VK_OEM_PERIOD, (unsigned)'.'},
};

static int tokey(unsigned key) {
	for(auto& e : sys_key_mapping_data) {
		if(e.key == key)
			return e.id;
	}
	return key;
}

static int handle(const MSG& msg) {
	switch(msg.message) {
	case WM_MOUSEMOVE:
		if(msg.hwnd != hwnd)
			break;
		hmouse.x = LOWORD(msg.lParam) / ZOOM;
		hmouse.y = HIWORD(msg.lParam) / ZOOM;
		return InputNoUpdate;
	case WM_LBUTTONDOWN:
		if(msg.hwnd != hwnd)
			break;
		hpressed = true;
		return MouseLeft;
	case WM_LBUTTONDBLCLK:
		if(msg.hwnd != hwnd)
			break;
		hpressed = true;
		return MouseLeftDBL;
	case WM_LBUTTONUP:
		if(msg.hwnd != hwnd)
			break;
		if(!hpressed)
			break;
		hpressed = false;
		return MouseLeft;
	case WM_RBUTTONDOWN:
		hpressed = true;
		return MouseRight;
	case WM_RBUTTONUP:
		hpressed = false;
		return MouseRight;
	case WM_MOUSEWHEEL:
		if(msg.wParam & 0x80000000)
			return MouseWheelDown;
		else
			return MouseWheelUp;
		break;
	case WM_MOUSEHOVER:
		return InputIdle;
	case WM_TIMER:
		if(msg.hwnd != hwnd)
			break;
		if(msg.wParam == InputTimer)
			return InputTimer;
		break;
	case WM_KEYDOWN:
		return tokey(msg.wParam);
	case WM_KEYUP:
		return InputKeyUp;
	case WM_CHAR:
		hparam = msg.wParam;
		return InputSymbol;
	case WM_MY_SIZE:
	case WM_SIZE:
		return InputUpdate;
	}
	return 0;
}

static LRESULT CALLBACK WndProc(HWND hwnd, unsigned uMsg, WPARAM wParam, LPARAM lParam) {
	MSG msg;
	RECT rc;
	switch(uMsg) {
	case WM_ERASEBKGND:
		GetClientRect(hwnd, &rc);
		video_descriptor.bmp.bmiHeader.biSize = sizeof(video_descriptor.bmp.bmiHeader);
		video_descriptor.bmp.bmiHeader.biWidth = video_buffer.width;
		video_descriptor.bmp.bmiHeader.biHeight = -video_buffer.height;
		video_descriptor.bmp.bmiHeader.biBitCount = video_buffer.bpp;
		video_descriptor.bmp.bmiHeader.biPlanes = 1;
		SetDIBitsToDevice((void*)wParam,
			0, 0, rc.right, rc.bottom,
			0, 0, 0, video_buffer.height,
			video_buffer.bits, &video_descriptor.bmp, DIB_RGB_COLORS);
		return 1;
	case WM_CLOSE:
		PostQuitMessage(-1);
		return 0;
	case WM_EXITSIZEMOVE:
	case WM_SIZE:
		if(!PeekMessageA(&msg, hwnd, WM_MY_SIZE, WM_MY_SIZE, 0))
			PostMessageA(hwnd, WM_MY_SIZE, 0, 0);
		return 0;
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = minimum.x;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = minimum.y;
		return 0;
	}
	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

static const char* register_class(const char* class_name) {
	WNDCLASS wc;
	if(!GetClassInfoA(GetModuleHandleA(0), class_name, &wc)) {
		memset(&wc, 0, sizeof(wc));
		wc.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW; // Own DC For Window.
		wc.lpfnWndProc = WndProc;
		wc.hInstance = GetModuleHandleA(0);
		wc.hIcon = (void*)LoadIconA(wc.hInstance, (const char*)1); // WndProc Handles Messages
		wc.lpszClassName = class_name;
		wc.hCursor = LoadCursorA(0, (char*)32512);
		RegisterClassA(&wc);
	}
	return class_name;
}

void sys_update_window() {
	if(!hwnd || !canvas)
		return;
	if(video_buffer.height != canvas->height * ZOOM
		|| video_buffer.width != canvas->width * ZOOM)
		video_buffer.resize(canvas->width * ZOOM, canvas->height * ZOOM, 32, true);
	scale2x(video_buffer.ptr(0, 0), video_buffer.scanline,
		canvas->ptr(0, 0), canvas->scanline,
		canvas->width, canvas->height);
	if(!IsWindowVisible(hwnd))
		ShowWindow(hwnd, SW_SHOW);
	InvalidateRect(hwnd, 0, 1);
	UpdateWindow(hwnd);
}

void sys_create_window(int width, int height) {
	unsigned dwStyle = WS_CAPTION | WS_SYSMENU | WS_BORDER; // Windows Style;
	auto client_width = width * ZOOM;
	auto client_height = height * ZOOM;
	RECT MinimumRect = {0, 0, client_width, client_height};
	AdjustWindowRectEx(&MinimumRect, dwStyle, 0, 0);
	auto window_width = MinimumRect.right - MinimumRect.left;
	auto window_height = MinimumRect.bottom - MinimumRect.top;
	auto x = (GetSystemMetrics(SM_CXFULLSCREEN) - window_width) / 2;
	auto y = (GetSystemMetrics(SM_CYFULLSCREEN) - window_height) / 2;
	minimum.x = client_width;
	minimum.y = client_height;
	if(canvas)
		canvas->resize(width, height, 32, true);
	setclip();
	// Create The Window
	hwnd = CreateWindowExA(0, register_class("LWUIWindow"), 0, dwStyle,
		x, y, window_width, window_height, 0, 0, GetModuleHandleA(0), 0);
	if(!hwnd)
		return;
	ShowWindow(hwnd, SW_SHOWNORMAL);
	// Update mouse coordinates
	POINT pt; GetCursorPos(&pt);
	ScreenToClient(hwnd, &pt);
	hmouse.x = (short)(pt.x / ZOOM);
	hmouse.y = (short)(pt.y / ZOOM);
}

static unsigned handle_event(unsigned m) {
	if(m < InputSymbol || m > InputNoUpdate) {
		if(GetKeyState(VK_SHIFT) < 0)
			m |= Shift;
		if(GetKeyState(VK_MENU) < 0)
			m |= Alt;
		if(GetKeyState(VK_CONTROL) < 0)
			m |= Ctrl;
	}
	return m;
}

void sys_input() {
	MSG	msg;
	sys_update_window();
	hkey = 0;
	if(!hwnd)
		return;
	while(GetMessageA(&msg, 0, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
		hkey = handle(msg);
		if(hkey == InputNoUpdate)
			continue;
		if(hkey) {
			hkey = handle_event(hkey);
			break;
		}
	}
}

void sys_redraw() {
	MSG	msg;
	sys_update_window();
	if(!hwnd)
		return;
	while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
		handle_event(handle(msg));
	}
}

void sys_caption(const char* string) {
	SetWindowTextA(hwnd, string);
}