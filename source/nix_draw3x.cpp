#include<X11/keysym.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include <cstdlib>
#include "draw.h"
#include "stringbuilder.h"

#define SCALE_FACTOR 2

const unsigned nix_event_mask = ExposureMask
| ButtonPressMask
| ButtonReleaseMask
| PointerMotionMask
| KeyPressMask
| KeyReleaseMask
| StructureNotifyMask
| VisibilityChangeMask
| LeaveWindowMask;
static Window       hwnd;
static Display*     dpy;
static GC           gc;
static int          scr;
static Window       rootwin;
static unsigned     timer_interval;

#ifdef SCALE_FACTOR
static char*         scaled_data;
void scale2x(void* void_dst, unsigned dst_slice, const void* void_src, unsigned src_slice, unsigned width, unsigned height);
#endif

static int tokey(int vk) {
	switch(vk) {
	case XK_Control_L:
	case XK_Control_R:
		return Ctrl;
	case XK_Alt_L:
	case XK_Alt_R:
		return Alt;
	case XK_Shift_L:
	case XK_Shift_R:
		return Shift;
	case XK_Left: case XK_KP_Left:
		return KeyLeft;
	case XK_Right: case XK_KP_Right:
		return KeyRight;
	case XK_Up: case XK_KP_Up:
		return KeyUp;
	case XK_Down: case XK_KP_Down:
		return KeyDown;
	case XK_Prior: case XK_KP_Page_Up:
		return KeyPageUp;
	case XK_Next: case XK_KP_Page_Down:
		return KeyPageDown;
	case XK_Home: case XK_KP_Home:
		return KeyHome;
	case XK_End: case XK_KP_End:
		return KeyEnd;
	case XK_BackSpace: case XK_KP_Space:
		return KeyBackspace;
	case XK_Delete:
		return KeyDelete;
	case XK_Return:
		return KeyEnter;
	case XK_Escape:
		return KeyEscape;
	case XK_space:
		return KeySpace;
	case XK_Tab: case XK_KP_Tab:
		return KeyTab;
	case XK_F1: case XK_KP_F4:
		return F1;
	case XK_F2:
		return F2;
	case XK_F3:
		return F3;
	case XK_F4:
		return F4;
	case XK_F5:
		return F5;
	case XK_F6:
		return F6;
	case XK_F7:
		return F7;
	case XK_F8:
		return F8;
	case XK_F9:
		return F9;
	case XK_F10:
		return F10;
	case XK_F11:
		return F11;
	case XK_F12:
		return F12;
		//case VK_MULTIPLY:
		//    return Alpha+(unsigned)'*';
		//case VK_DIVIDE:
		//    return Alpha+(unsigned)'/';
	case XK_plus:
		return '+';
	case XK_minus:
		return '-';
	case XK_comma:
		return ',';
	case XK_period:
		return '.';
	default:
		return upper_symbol(vk);
	}
}

const char* key2str(int key) {
	static char temp[128];
	temp[0] = 0;
	return temp;
}

static void widget_cleanup() {
	XCloseDisplay(dpy);
}

void sys_timer(unsigned v) {
	timer_interval = v;
}

void sys_create_window(int width, int height) {
	if(hwnd)
		return;
	XSetWindowAttributes attr = {};
	attr.background_pixmap = None;
	attr.backing_store = NotUseful;
	dpy = XOpenDisplay(0);
	scr = DefaultScreen(dpy);
	rootwin = RootWindow(dpy, scr);
	atexit(widget_cleanup);
	Visual* myVisual = DefaultVisual(dpy, scr);
	if(canvas)
		canvas->resize(width, height, 32, true);
	setclip();
#ifdef SCALE_FACTOR
	scaled_data = new char[canvas->scanline * SCALE_FACTOR * canvas->width * SCALE_FACTOR];
	hwnd = XCreateWindow(dpy, rootwin, -1, -1, width * SCALE_FACTOR, height * SCALE_FACTOR, 0, 24, InputOutput, myVisual, CWBackPixmap | CWBackingStore, &attr);
#else
	hwnd = XCreateWindow(dpy, rootwin, -1, -1, width, height, 0, 24, InputOutput, myVisual, CWBackPixmap | CWBackingStore, &attr);
#endif // SCALE_FACTOR
	gc = XCreateGC(dpy, hwnd, 0, NULL);
	XSelectInput(dpy, hwnd, nix_event_mask);
	XMapWindow(dpy, hwnd);
}

static void update_ui_window() {
	if(!canvas)
		return;
	XImage img = {0};
	img.format = ZPixmap;
	img.bitmap_unit = 32;
	img.bitmap_pad = 32;
	img.depth = 24;
	img.bits_per_pixel = 32;
#ifdef SCALE_FACTOR
	img.width = canvas->width * SCALE_FACTOR;
	img.height = canvas->height * SCALE_FACTOR;
	img.data = scaled_data;
	scale2x(scaled_data, canvas->scanline * SCALE_FACTOR,
      canvas->bits, canvas->scanline,
      canvas->width, canvas->height);
#else
	img.width = canvas->width;
	img.height = canvas->height;
	img.data = (char*)canvas->ptr(0,0);
#endif // SCALE_FACTOR
	if(XInitImage(&img))
		XPutImage(dpy, hwnd, gc, &img, 0, 0, 0, 0, img.width, img.height);
}

static int mousekey(int k, bool dbl = false) {
	switch(k) {
	case Button1:
		return dbl ? MouseLeftDBL : MouseLeft;
	case Button3:
		return MouseRight;
	case Button4:
		return MouseWheelUp;
	case Button5:
		return MouseWheelDown;
	default:
		return 0;
	}
}

static void apply_keyboard_mask(unsigned mask) {
	if((mask & ShiftMask) != 0)
		hkey |= Shift;
	if((mask & ControlMask) != 0)
		hkey |= Ctrl;
	if((mask & LockMask) != 0)
		hkey |= Alt;
}

static bool handle(XEvent& e) {
	XEvent e1;
	static unsigned long last_mouse_press;
	switch(e.type) {
	case Expose:
		hkey = InputUpdate;
		break;
	case ButtonPress:
		hkey = mousekey(e.xbutton.button, (e.xbutton.time - last_mouse_press) < 300);
		apply_keyboard_mask(e.xbutton.state);
		hpressed = true;
		last_mouse_press = e.xbutton.time;
		break;
	case ButtonRelease:
		hkey = mousekey(e.xbutton.button);
		apply_keyboard_mask(e.xbutton.state);
		hpressed = false;
		break;
	case KeyPress:
		hkey = tokey(XkbKeycodeToKeysym(dpy, e.xkey.keycode, 0, e.xkey.state & ShiftMask ? 1 : 0));
		apply_keyboard_mask(e.xkey.state);
		break;
	case KeyRelease:
		hkey = InputKeyUp;
		apply_keyboard_mask(e.xkey.state);
		break;
	case MotionNotify:
		while(XCheckTypedWindowEvent(dpy, e.xmotion.window, e.type, &e1));
		hkey = MouseMove;
		hmouse.x = e.xmotion.x;
		hmouse.y = e.xmotion.y;
#ifdef SCALE_FACTOR
		hmouse.x /= SCALE_FACTOR;
		hmouse.y /= SCALE_FACTOR;
#endif // SCALE_FACTOR
		break;
	case VisibilityNotify:
		hkey = InputUpdate;
		break;
	case ConfigureNotify:
		if(e.xconfigure.width != getwidth() || e.xconfigure.height != getheight()) {
			while(XCheckTypedWindowEvent(dpy, e.xconfigure.window, e.type, &e1));
#ifdef SCALE_FACTOR
			canvas->resize(e.xconfigure.width / SCALE_FACTOR, e.xconfigure.height / SCALE_FACTOR, 32, true);
#else
			canvas->resize(e.xconfigure.width, e.xconfigure.height, 32, true);
#endif // SCALE_FACTOR
			setclip();
			hkey = InputUpdate;
		} else
			return false;
		break;
	case LeaveNotify:
		hkey = MouseMove;
		hpressed = false;
		hmouse.x = -1000;
		hmouse.y = -1000;
		break;
	case DestroyNotify:
		hkey = 0;
		break;
	default:
		return false;
	}
	return true;
}

void sys_update_window() {
	if(!hwnd)
		return;
	update_ui_window();
	XEvent e;
	while(XCheckWindowEvent(dpy, hwnd, nix_event_mask, &e))
		handle(e);
}

void sys_redraw() {
   sys_update_window();
}

void sys_input() {
	if(!hwnd)
		return;
	update_ui_window();
	while(true) {
		XEvent e;
		XNextEvent(dpy, &e);
		if(XFilterEvent(&e, hwnd))
			continue;
		if(!handle(e))
			continue;
		break;
	}
}

void sys_caption(const char* format) {
	Xutf8SetWMProperties(dpy, hwnd, format, 0, 0, 0, 0, 0, 0);
}
