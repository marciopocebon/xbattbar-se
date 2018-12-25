/*
 * xbattbar-se: yet another battery watcher for X11 (suckless edition)
 *
 * Copyright (c) 1998-2001 Suguru Yamaguchi <suguru@wide.ad.jp>
 * Copyright (c) 2018      Alexandr Savca   <alexandr.savca89@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

static char *ReleaseVersion = "0.1.2";

#include <X11/Xlib.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define BI_Bottom	0
#define BI_Top		1
#define BI_Left		2
#define BI_Right	3
#define BI_Horizontal	((bi_direction & 2) == 0)
#define BI_Vertical	((bi_direction & 2) == 2)

#define myEventMask	(  ExposureMask		\
			 | EnterWindowMask	\
			 | LeaveWindowMask	\
			 | VisibilityChangeMask	)

#include "config.h"


/*
 * Global variables
 * */
static int first = 1;
static int ac_line = -1;		/* AC line status */
static int battery_level = -1;		/* battery level */

static unsigned long on_in, on_out;	/* indicator colors for AC online */
static unsigned long off_in, off_out;	/* indicator colors for AC offline */

static int elapsed_time = 0;		/* for battery remaining estimation */

typedef struct itimerval iTimerVal;	/* polling interval timer */
static iTimerVal IntervalTimer;

static int bi_height;			/* height of Battery Indicator */
static int bi_width;			/* width of Battery Indicator */
static int bi_x;			/* x coordinate of upper left corner */
static int bi_y;			/* y coordinate of upper left corner */

static Display *disp;
static Window winbar;			/* bar indicator window */

static GC gcbar;
static unsigned int width, height;
static XEvent theEvent;

/*
 * Function prototypes
 * */
static void InitDisplay(void);
static Status AllocColor(char *name, unsigned long *pixel);
static void battery_check(void);
static void plug_proc(int left);
static void battery_proc(int left);
static void redraw(void);
static void about_this_program(void);
static void estimate_remain(void);
static void die(const char *errstr, ...);

/*
 * Usage of this command
 * */
void about_this_program()
{
	fprintf(stderr,
		"This is xbattbar-pe version %s\n"
		"copyright (c) 1998-2001 Sugumaru Yamaguchi\n"
		"copyright (c) 2018      Alexandr Savca\n",
		ReleaseVersion);
}

/*
 * Die and exit
 * */
void die(const char *errstr, ...)
{
	va_list ap;

	fprintf(stderr, "xbattbar: ");
	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	fprintf(stderr, ".\n");
	exit(EXIT_FAILURE);
}

/*
 * AllocColor:
 *      convert color name to pixel value
 * */
Status AllocColor(char *name, unsigned long *pixel)
{
	XColor  color, exact;
	int     status;

        status = XAllocNamedColor(disp,
				DefaultColormap(disp, 0),
				name,
				&color,
				&exact);
        *pixel = color.pixel;

        return status;
}

/*
 * InitDisplay:
 *      create small window in top or bottom
 * */
void InitDisplay(void)
{
	int                   x, y;
        unsigned int          border, depth;
        Window                root;
        XSetWindowAttributes  att;

	if ((disp = XOpenDisplay(NULL)) == NULL)
		die("can't open display");

	if (XGetGeometry(disp,
			DefaultRootWindow(disp),
			&root,
			&x,
			&y,
			&width,
			&height,
			&border,
			&depth) == 0)
		die("can't get window geometry");

	if (   !AllocColor(ONIN_C,   &on_in)
	    || !AllocColor(ONOUT_C,  &on_out)
	    || !AllocColor(OFFIN_C,  &off_in)
	    || !AllocColor(OFFOUT_C, &off_out) )
		die("can't allocate color resources");

	switch (bi_direction) {
	case BI_Top: /* (0, 0) - (width, bi_thick) */
		bi_width  = width;
		bi_height = bi_thick;
		bi_x = 0;
		bi_y = 0;
		break;

	case BI_Bottom:
		bi_width  = width;
		bi_height = bi_thick;
		bi_x = 0;
		bi_y = height - bi_thick;
		break;

	case BI_Left:
		bi_width  = bi_thick;
		bi_height = height;
		bi_x = 0;
		bi_y = 0;
		break;

	case BI_Right:
		bi_width  = bi_thick;
		bi_height = height;
		bi_x = width - bi_thick;
		bi_y = 0;
	}

	winbar = XCreateSimpleWindow(disp,
				DefaultRootWindow(disp),
				bi_x,
				bi_y,
				bi_width,
				bi_height,
				0,
				BlackPixel(disp, 0),
				WhitePixel(disp, 0));

	/* make this window without its titlebar */
	att.override_redirect = True;
	XChangeWindowAttributes(disp, winbar, CWOverrideRedirect, &att);
	XMapWindow(disp, winbar);
	gcbar = XCreateGC(disp, winbar, 0, 0);
}

void redraw(void)
{
	if (ac_line)
		plug_proc(battery_level);
	else
		battery_proc(battery_level);

	estimate_remain();
}

void battery_proc(int left)
{
	int pos;
	if (BI_Horizontal) {
		pos = width * left / 100;
		XSetForeground(disp, gcbar, off_in);
		XFillRectangle(disp, winbar, gcbar, 0, 0, pos, bi_thick);
		XSetForeground(disp, gcbar, off_out);
		XFillRectangle(disp, winbar, gcbar, pos, 0, width, bi_thick);
	} else {
		pos = height * left / 100;
		XSetForeground(disp, gcbar, off_in);
		XFillRectangle(disp, winbar, gcbar, 0, height-pos, bi_thick,
				height);
		XSetForeground(disp, gcbar, off_out);
		XFillRectangle(disp, winbar, gcbar, 0, 0, bi_thick, height-pos);
	}
	XFlush(disp);
}

void plug_proc(int left)
{
	int pos;
	if (BI_Horizontal) {
		pos = width * left / 100;
		XSetForeground(disp, gcbar, on_in);
		XFillRectangle(disp, winbar, gcbar, 0, 0, pos, bi_thick);
		XSetForeground(disp, gcbar, on_out);
		XFillRectangle(disp, winbar, gcbar, pos+1, 0, width, bi_thick);
        } else {
		pos = height * left / 100;
		XSetForeground(disp, gcbar, on_in);
		XFillRectangle(disp, winbar, gcbar, 0, height-pos, bi_thick,
				height);
		XSetForeground(disp, gcbar, on_out);
		XFillRectangle(disp, winbar, gcbar, 0, 0, bi_thick, height-pos);
	}
	XFlush(disp);
}

/*
 * estimating time for battery remaining / charging
 * */
#define CriticalLevel 5
void estimate_remain()
{
	static int  battery_base = -1;
	int         diff;
	int         remain;

	/* static value initialize */
	if (battery_base == -1) {
		battery_base = battery_level;
		return;
	}

	diff = battery_base - battery_level;

	if (diff <= 0)
		return;

	/* estimated time for battery remains */
	if (diff > 0) {
		remain = elapsed_time * (battery_level - CriticalLevel) / diff;
		remain = remain * bi_interval;  /* in sec */
		if (remain < 0 )
			remain = 0;

		printf("battery remain: %2d hr. %2d min. %2d sec.\n",
			remain / 3600, (remain % 3600) / 60, remain % 60);

		elapsed_time = 0;
		battery_base = battery_level;
		return;
	}

	/* estimated time of battery charging */
	remain = elapsed_time * (battery_level - 100) / diff;
	remain = remain * bi_interval;  /* in sec */
	printf("charging remain: %2d hr. %2d min. %2d sec.\n",
		remain / 3600, (remain % 3600) / 60, remain % 60);
	elapsed_time = 0;
	battery_base = battery_level;
}

void battery_check(void)
{
	FILE *pt;
	int r, p, f, n;

	if ((pt = fopen(BattFull, "r")) == NULL) {
		perror("xbattbar: cannot open "BattFull);
		exit(1);
	}

	fscanf(pt, "%d", &f);
	fclose(pt);

	if ((pt = fopen(BattNow, "r")) == NULL) {
		perror("xbattbar: cannot open "BattNow);
		exit(1);
	}

	fscanf(pt, "%d", &n);
	fclose(pt);

	if ((r = (int)(n / (float)f * 100)) > 100)
		r = 100;

	if ((pt = fopen(PowerState, "r")) == NULL) {
		perror("xbattbar: cannot open "PowerState);
		exit(1);
	}

	fscanf(pt, "%d", &p);
	fclose(pt);

	++elapsed_time;

	if (first || ac_line != p || battery_level != r) {
		first = 0;
		ac_line = p;
		battery_level = r;
		redraw();
	}

	signal(SIGALRM, (__sighandler_t)(battery_check));
}

int main(int argc, char **argv)
{
	if (argc == 2 && !strcmp("-v", argv[1]))
		about_this_program();
	else if (argc != 1)
		die("usage: %s [-v]", argv[0]);

        /*
         * set polling interval timer
         */
	IntervalTimer.it_interval.tv_sec  = (long)bi_interval;
	IntervalTimer.it_interval.tv_usec = (long)0;
	IntervalTimer.it_value.tv_sec     = (long)1;
	IntervalTimer.it_value.tv_usec    = (long)0;
	if (setitimer(ITIMER_REAL, &IntervalTimer, NULL) != 0)
		die("can't set interval timer");

        /*
         * X Window main loop
         */
	InitDisplay();
	signal(SIGALRM, (__sighandler_t)(battery_check));
	battery_check();
	XSelectInput(disp, winbar, myEventMask);
	while (1) {
		XWindowEvent(disp, winbar, myEventMask, &theEvent);
		switch (theEvent.type) {
		case Expose:
			/* we redraw our window since our window has been exposed */
			redraw();
			break;

		case VisibilityNotify:
			if (always_on_top)
				XRaiseWindow(disp, winbar);
			break;

		default:
			/* for debugging */
			fprintf(stderr,
				"xbattbar: unknown event (%d) captured\n",
				theEvent.type);
		}
	}
	return EXIT_SUCCESS;
}

/* vim: set cc=80 : sw=8 : ts=8 : et
 * End of file
 * */
