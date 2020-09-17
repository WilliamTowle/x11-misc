/*	xtest.c
 *	vim: ft=c ts=4 sw=4 et:
 *
 *   Open Source software - copyright and GPLv2 apply. Briefly:       *
 *    - No warranty/guarantee of fitness, use is at own risk          *
 *    - No restrictions on strictly-private use/copying/modification  *
 *    - No re-licensing this work under more restrictive terms        *
 *    - Redistributing? Include/offer to deliver original source      *
 *   Philosophy/full details at http://www.gnu.org/copyleft/gpl.html  *
 *
 *   Bitmap images (c) 1990 Masayuki Koba according to txneko
 */

#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>

/* 'bitmaps' tree includes "dog", "neko" (cat), "sakura", "tomoyo", "tora" */
/* bitmaps include
 *  awake   - alarmed (...movement follows)
 *  {down|up}<n>, dw{left|right}<n>, {left|right}<n>, up{left|right}<n> - moving
 *  dtogi<n>, ltogi<n>, rtogi<n>, utogi<n> - pawing
 *  jare2 - washing
 *  kaki<n> - scratching
 *  mati2 - sitting
 *  mati3 - yawning
 *  sleep<n>
 */
/* see also "bitmaps/neko/include" */
#include "bitmaps/neko/awake.xbm"	/* awake_{width,height,bits} */

#define DEFAULT_DISPLAY ":0"

#define BITMAP_HEIGHT	awake_height
#define BITMAP_WIDTH	awake_width

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
			BITMAP_WIDTH, BITMAP_HEIGHT,        /* w,h */
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

static void draw()
{
    Pixmap      anim_xbm;
    GC          anim_GC;
    XGCValues   anim_GCValues;

	anim_xbm= XCreateBitmapFromData(config.xdpy, config.xroot,
		awake_bits, BITMAP_WIDTH, BITMAP_HEIGHT
        );

	anim_GCValues.function= GXcopy;
	anim_GCValues.fill_style = FillTiled;
	anim_GCValues.ts_x_origin = 0;
	anim_GCValues.ts_y_origin = 0;
	anim_GCValues.tile= anim_xbm;

    anim_GC= XCreateGC(config.xdpy, config.xroot, 0, &anim_GCValues);
    XSetForeground(config.xdpy, anim_GC, BlackPixel(config.xdpy, config.xscr));
    XSetBackground(config.xdpy, anim_GC, WhitePixel(config.xdpy, config.xscr));

    XCopyPlane(config.xdpy, anim_xbm, config.mainwin, anim_GC,
            0,0, BITMAP_WIDTH, BITMAP_HEIGHT,   /* source region */
            0,0, 1                              /* dest region/plane */
            );
    XSync(config.xdpy, False);

	XFlush(config.xdpy);
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

    draw();
    sleep(5);

    return gui_fini();
}
