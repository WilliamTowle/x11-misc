/*	xtest.c
 *	vim: ft=c ts=4 sw=4 et:
 *
 *   Open Source software - copyright and GPLv2 apply. Briefly:       *
 *    - No warranty/guarantee of fitness, use is at own risk          *
 *    - No restrictions on strictly-private use/copying/modification  *
 *    - No re-licensing this work under more restrictive terms        *
 *    - Redistributing? Include/offer to deliver original source      *
 *   Philosophy/full details at http://www.gnu.org/copyleft/gpl.html  *
 */

#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>

#define WIN_WIDTH	100
#define WIN_HEIGHT	50
#define DEFAULT_DISPLAY ":0"


Display *xdpy;
int xscr;
Window xroot;
Window mainwin;


static int gui_init(void)
{
    const char *dispname= getenv("DISPLAY");

    if (dispname == NULL)
        dispname= DEFAULT_DISPLAY;

	xdpy= XOpenDisplay(dispname);

	xscr= DefaultScreen(xdpy);
	xroot= RootWindow(xdpy, xscr);

	mainwin= XCreateSimpleWindow(xdpy, xroot,
			10, 10,                     /* x,y */
			WIN_WIDTH, WIN_HEIGHT,	/* w,h */
			1, BlackPixel(xdpy, xscr),  /* window's border */
			WhitePixel(xdpy, xscr));    /* window's background */
    if (!mainwin)
        return EXIT_FAILURE;

    /* register for our MapNotify event */
	XSelectInput(xdpy, mainwin, StructureNotifyMask);

	XMapWindow(xdpy, mainwin);

    /* await our MapNotify event */
    while (1)
    {
        XEvent xe;
        XNextEvent(xdpy, &xe);
        if (xe.type == MapNotify)
            break;
    }

        XClearWindow(xdpy, mainwin);

    return EXIT_SUCCESS;
}

static int gui_fini(void)
{
	XDestroyWindow(xdpy, mainwin);

    return XCloseDisplay(xdpy);
}

int main()
{
    if (gui_init() != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    sleep(5);

    return gui_fini();
}
