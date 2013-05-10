// File: Xoutput.cc
// Date: Sun Aug 26 22:17:48 2012 +0800
// Author: Yuxin Wu <ppwwyyxxc@gmail.com>

#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <cmath>
#include <cassert>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include "Xoutput.h"
#include "pngwriter.h"
#include "calculate.h"
#include <omp.h>
using namespace std;

#define ZOOM_RATIO 1.3

Display *dpy;
Window win;
GC gc;
Atom wmDeleteMessage;
extern double xmin, ymin, xlen, ylen;
extern int SIZE_X, SIZE_Y;
extern short* img;
extern int myid;
extern int maxiter;
int done = 0;

void initialized_window(){
	const char *window_name = "Mandelbrot Set";
	if ((dpy = XOpenDisplay(NULL)) == NULL ){
		cerr << "cannot connect to X server \n";
		exit(-1);
	}

	int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
	int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

	win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 
			SIZE_X, SIZE_Y, 0, whiteColor, whiteColor);
	XStoreName(dpy, win, window_name);
	XSelectInput(dpy, win, StructureNotifyMask | KeyPressMask | ButtonPressMask);

   // register interest in the delete window message
   wmDeleteMessage = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
   XSetWMProtocols(dpy, win, &wmDeleteMessage, 1);

	XMapWindow(dpy, win);
	gc = XCreateGC(dpy, win, 0, NULL);
	XSetForeground(dpy, gc, blackColor);
	for(;;) {
		XEvent e;
		XNextEvent(dpy, &e);
		if (e.type == MapNotify)
			break;
	}
}

void draw_point(int x, int y, short depth){
	short t = depth;
	int color = (65536 * t+ 256 * t + t);
	XSetForeground(dpy, gc, color);
	XDrawPoint(dpy, win, gc, x, y);
}

void draw_label(){
	char buffer[100];
	XSetForeground(dpy, gc, 255 * 65536);
	int border = 10;
	if ((SIZE_X < 100) || (SIZE_Y < 50)) return;

#define DRAWSTRING(a, b) XDrawString(dpy, win, gc, a, b, buffer, strlen(buffer))
	sprintf(buffer, "(%e, %e)", xmin, ymin);
	DRAWSTRING(border, 2 * border);
	sprintf(buffer, "len_x = %e, len_y = %e", xlen, ylen);
	DRAWSTRING(border, 4 * border);
#undef DRAWSTRING
}

void X_drawimg(){
	for (int i = 0; i < SIZE_X; i ++)
		for (int j = 0; j < SIZE_Y; j ++)
			draw_point(i, j, img[i * SIZE_Y + j]);

	draw_label();
}

void move(int dir){
	print_debug("in move");
	double shift = (xlen + ylen) / 20;
	switch (dir){
		case XK_Up:
			ymin += shift;
			break;
		case XK_Down:
			ymin -= shift;
			break;
		case XK_Left:
			xmin += shift;
			break;
		case XK_Right:
			xmin -= shift;
			break;
		default:
			break;
	}
}

void move(int x_c, int y_c, int button){
	double ratio;
	if (button == Button3)
		ratio = 1 / ZOOM_RATIO;
	else if (button == Button1)
		ratio = ZOOM_RATIO;
	else 
		return;
	double xc = x_c * xlen / SIZE_X + xmin, yc = y_c * ylen / SIZE_Y + ymin;
	xlen = xlen / ratio, ylen = ylen / ratio;
	xmin = xc - xlen / 2, ymin = yc - ylen / 2;
}

void zoom(int dir, double ratio = ZOOM_RATIO){
	if (dir == XK_minus)
		ratio = 1 / ratio;
	double xc = xmin + xlen / 2, yc = ymin + ylen / 2;
	xlen = xlen / ratio, ylen = ylen / ratio;
	xmin = xc - xlen / 2, ymin = yc - ylen / 2;
}

void X_wait_event(){
	XEvent e;
	XNextEvent(dpy, &e);
	switch (e.type) {
		case KeyPress:
			{
				int tmp = XLookupKeysym(&e.xkey, 0);
				switch (tmp){
					case XK_Up:
					case XK_Down:
					case XK_Left:
					case XK_Right:
						move(tmp);
#ifndef USE_MPI
						cal_rectangle();
						X_drawimg();
#endif
						break;
					case XK_minus:
					case XK_equal:
						zoom(tmp);
#ifndef USE_MPI
						cal_rectangle();
						X_drawimg();
#endif
						break;
					case XK_s:
						{
							string str;
							cout << "Please Enter File Name: *.png" << endl;
							cin >> str;
							write_image(str.c_str(), img, SIZE_X, SIZE_Y);
							break;
						}
					case XK_c:
						{
							int newiter;
							cout << "Please Enter Max Iteration Times: " << endl;
							cin >> newiter;
							maxiter = newiter;
#ifndef USE_MPI
							cal_rectangle();
							X_drawimg();
#endif
							break;
						}
					case XK_Escape:
						done = 1;
						break;
					default:
						break;
				}
				break;
			}
		case ConfigureNotify:
			{
				XConfigureEvent xce = e.xconfigure;
				if ((xce.width != SIZE_X) || (xce.height != SIZE_Y)){
					xlen = xlen * xce.width / SIZE_X, ylen = ylen * xce.height / SIZE_Y;
					SIZE_X = xce.width, SIZE_Y = xce.height;
					delete[] img;
					img = new short [SIZE_X * SIZE_Y];
#ifndef USE_MPI
					cal_rectangle();
					X_drawimg();
#endif
				}
				break;
			}
		case ButtonPress:
			{
				int x = e.xbutton.x, y = e.xbutton.y;
				move(x, y, e.xbutton.button);
#ifndef USE_MPI
				cal_rectangle();
				X_drawimg();
#endif
			}
		case ClientMessage:
			if (e.xclient.data.l[0] == wmDeleteMessage)
				done = true;
			break;
		default:
			break;
	}
}

void X_wait(){
	while (!done){
#ifdef USE_MPI
		if (myid == 0){
#endif
			X_wait_event();
#ifdef USE_MPI
		}
		MPI_Barrier(MPI_COMM_WORLD);
		MPI_Bcast(&done, 1, MPI_INT, 0, MPI_COMM_WORLD);
		if (done)
			return;
		bcast(xmin);bcast(ymin);bcast(xlen);bcast(ylen);
		MPI_Bcast(&maxiter, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&SIZE_X, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&SIZE_Y, 1, MPI_INT, 0, MPI_COMM_WORLD);
		cal_rectangle();
		if (myid == 0)
			X_drawimg();
#endif
	}
}

