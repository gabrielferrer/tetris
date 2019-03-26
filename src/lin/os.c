#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include "os.h"
#include "game.h"
#include "render.h"

#define DEPTH 24
#define QUANTUM 32

Display* Dsp;
Window Wnd;
int Scr;
GC Gc;
XImage* Image;
Visual* Vsl;
XVisualInfo VisualInfo;
XSetWindowAttributes Attribs;
extern unsigned int* Scene;

int OS_MapKey (int key) {
	switch (key) {
		case XK_Left:
			return KEY_LEFT;
		case XK_Right:
			return KEY_RIGHT;
		case XK_Up:
			return KEY_UP;
		case XK_Down:
			return KEY_DOWN;
		case XK_space:
			return KEY_SPACE;
		case XK_s:
		case XK_S:
			return KEY_S;
		case XK_p:
		case XK_P:
			return KEY_P;
		default:
			return KEY_NONE;
	}
}

void OS_HandleKeyPress (unsigned int keycode) {
	GM_SetKey (OS_MapKey (XkbKeycodeToKeysym (Dsp, keycode, 0, 0)));
}

/*
	Returns time tick in microseconds.
*/
long OS_GetTick () {
    struct timeval timeVal;
    gettimeofday (&timeVal, NULL);
    return (long) (timeVal.tv_sec * 1000000 + timeVal.tv_usec);
}

void OS_Sleep (long milisecs) {
	struct timespec reqtime;
	struct timespec remaining;

	reqtime.tv_sec = milisecs / 1000;
	reqtime.tv_nsec = milisecs * 1000000;

	nanosleep (&reqtime, &remaining);
}

void OS_Exit (char* function, char* failfunction) {
	printf ("%s(): %s() failed", function, failfunction);
	exit (EXIT_FAILURE);
}

void OS_Init () {
	unsigned long attribmask, valuemask;
	long infomask;
	Window rootwnd;
	XGCValues gcvalues;

	if ((Dsp = (Display*) XOpenDisplay (NULL)) == NULL) {
		OS_Exit ("OS_Init", "XOpenDisplay");
	}

	Scr = XDefaultScreen (Dsp);
	rootwnd = XRootWindow (Dsp, Scr);

	if (!XMatchVisualInfo (Dsp, Scr, DEPTH, TrueColor, &VisualInfo)) {
		OS_Exit ("OS_Init", "XMatchVisualInfo");
	}

	Vsl = VisualInfo.visual;
	Vsl = (Visual*) XDefaultVisual (Dsp, Scr);

	attribmask = CWEventMask | CWBorderPixel;
	Attribs.border_pixel = 0;
	Attribs.event_mask = KeyPressMask | ExposureMask;

	if ((Wnd = XCreateWindow (Dsp, rootwnd, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, DEPTH, InputOutput, Vsl, attribmask, &Attribs)) <= 0) {
		OS_Exit ("OS_Init", "XCreateWindow");
	}

	valuemask = GCGraphicsExposures;
	gcvalues.graphics_exposures = True;

	Gc = (GC) XCreateGC (Dsp, Wnd, valuemask, &gcvalues);

	if ((Image = (XImage*) XCreateImage (Dsp, Vsl, DEPTH, ZPixmap, 0, NULL, WINDOW_WIDTH, WINDOW_HEIGHT, QUANTUM, 0)) == NULL) {
		OS_Exit ("OS_Init", "XCreateImage");
	}

	if ((Scene = (unsigned int*) malloc (Image->bytes_per_line * Image->height)) == NULL) {
		OS_Exit ("OS_Init", "malloc");
	}

	// Make compiler happy.
	Image->data = (char*) Scene;

	XMapWindow (Dsp, Wnd);
}

void OS_ProcessEvents () {
	XEvent ev;

	if (!XCheckWindowEvent (Dsp, Wnd, KeyPressMask | ExposureMask, &ev)) {
		return;
	}

	switch (ev.type) {
		case KeyPress:
			OS_HandleKeyPress (ev.xkey.keycode);
			break;
		case Expose:
			break;
		default:
			break;
	}
}

void OS_Render() {
	XPutImage (Dsp, Wnd, Gc, Image, 0, 0, 0, 0, Image->width, Image->height);
}

void OS_Terminate() {
	XCloseDisplay (Dsp);

	if (Image) {
		/* This destroys Imaga->data too because was XCreateImage() was used */
		XDestroyImage (Image);
	}
}
