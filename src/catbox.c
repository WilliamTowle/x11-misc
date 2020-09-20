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


struct {
    const char  *progname;
    Display     *xdpy;
    int         xscr;
    Window      xroot;
    Window      mainwin;
} config;


static int gui_init(void)
{
    const char *dispname= getenv("DISPLAY");

    if (dispname == NULL)
        dispname= DEFAULT_DISPLAY;

	config.xdpy= XOpenDisplay(dispname);

    config.xscr= DefaultScreen(config.xdpy);
	config.xroot= RootWindow(config.xdpy, config.xscr);

	config.mainwin= XCreateSimpleWindow(config.xdpy, config.xroot,
			10, 10,                     /* x,y */
			WIN_WIDTH, WIN_HEIGHT,	/* w,h */
			1, BlackPixel(config.xdpy, config.xscr),  /* window's border */
			WhitePixel(config.xdpy, config.xscr));    /* window's background */
    if (!config.mainwin)
        return EXIT_FAILURE;

    /* register for our MapNotify event */
	XSelectInput(config.xdpy, config.mainwin, StructureNotifyMask);

	XMapWindow(config.xdpy, config.mainwin);

    /* await our MapNotify event */
    while (1)
    {
        XEvent xe;
        XNextEvent(config.xdpy, &xe);
        if (xe.type == MapNotify)
            break;
    }

    XClearWindow(config.xdpy, config.mainwin);

    return EXIT_SUCCESS;
}

static int gui_fini(void)
{
	XDestroyWindow(config.xdpy, config.mainwin);

    return XCloseDisplay(config.xdpy);
}

int main(const int argc, const char **argv)
{
    config.progname= argv[0];

    if (gui_init() != EXIT_SUCCESS)
    {
        printf("%s: initialisation failed\n", config.progname);
        return EXIT_FAILURE;
    }

    sleep(5);

    return gui_fini();
}
