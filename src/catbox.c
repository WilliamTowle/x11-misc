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

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
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
#include "bitmaps/neko/jare2.xbm"
#include "bitmaps/neko/mati2.xbm"
#include "bitmaps/neko/sleep1.xbm"
#include "bitmaps/neko/sleep2.xbm"
#include "bitmaps/dog/awake_dog.xbm"
#include "bitmaps/dog/jare2_dog.xbm"
#include "bitmaps/dog/mati2_dog.xbm"
#include "bitmaps/dog/sleep1_dog.xbm"
#include "bitmaps/dog/sleep2_dog.xbm"

#define DEFAULT_DISPLAY ":0"

#define BITMAP_HEIGHT	bitmap_table[config.character].height
#define BITMAP_WIDTH	bitmap_table[config.character].width

#define MAX_TICK        1000

struct {
    const char      *progname;
    Display         *xdpy;
    int             xscr;
    Window          xroot;

    Window          mainwin;
    unsigned int    character;
    int             anim_tick_count;
    int             anim_state_count;
} config;

struct {
        const char  *name;
        int         width;
        int         height;
        long        time;
        char        *awake_bits,
                    *mati_bits, *jare_bits,
                    *sleep1_bits, *sleep2_bits;
} bitmap_table[] = {
        { "neko",   awake_width, awake_height, 125000L,
                    awake_bits,
                    mati2_bits, jare2_bits,
                    sleep1_bits, sleep2_bits },
        { "dog",    awake_dog_width, awake_dog_height, 125000L,
                    awake_bits,
                    mati2_dog_bits, jare2_dog_bits,
                    sleep1_dog_bits, sleep2_dog_bits },
        { NULL,     0,0, 0L,
                    NULL, NULL, NULL, NULL, NULL }
};

typedef enum {
    ANIM_STOP,      /* sitting - frame[s]: mati2 (sitting) */
    ANIM_JARE,      /* resting - frame[s]: jare2 (scratching/washing) */
    ANIM_SLEEP,     /* asleep - frame[s]: sleep1/sleep2 (snoring) */
    ANIM_STATE_LAST
} anim_state_t;

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

    /* register for initial MapNotify event */
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

static void draw(int patnum)
{
    char        *anim_bits[]= { bitmap_table[config.character].awake_bits,
                                bitmap_table[config.character].mati_bits,
                                bitmap_table[config.character].jare_bits,
                                bitmap_table[config.character].sleep1_bits,
                                bitmap_table[config.character].sleep2_bits };
    Pixmap      anim_xbm;
    GC          anim_GC;
    XGCValues   anim_GCValues;

    if (patnum >= 5)
        patnum= 0;  /* force "awake" if out of range */

	anim_xbm= XCreateBitmapFromData(config.xdpy, config.xroot,
		anim_bits[patnum], BITMAP_WIDTH, BITMAP_HEIGHT
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

static void sighandler_alrm()   /* passed int, ignored */
{
    if (++config.anim_tick_count >= MAX_TICK)
        config.anim_tick_count= 0;

    if (config.anim_tick_count % 2 == 0)
        if (config.anim_state_count < MAX_TICK)
            config.anim_state_count++;
}

static void sighandler_usr(int signum)
{
    unsigned int characters;
    int new_character;

    characters= 0;
    while (bitmap_table[characters].name != NULL)
        characters++;

    new_character= config.character + (signed int)((signum == SIGUSR1)?1:-1);
    if (new_character < 0)
    {
        new_character = characters - 1;
    }
    else if (new_character >= (int)characters)
    {
        new_character= 0;
    }

    config.character= new_character;
}

static void run_animation()
{
    anim_state_t        curr_anim_state, old_anim_state;
    int                 mouse_in_window, old_mouse_in_window;
    long                interval_time= bitmap_table[config.character].time;
    struct itimerval	itimerval;

    curr_anim_state= ANIM_STOP;
    mouse_in_window= 0;
    config.anim_tick_count= 0;
    config.anim_state_count= 0;

    /* Change events we get queued */
	XSelectInput(config.xdpy, config.mainwin, FocusChangeMask );

    do
    {
        XEvent  xe;

        old_anim_state= curr_anim_state;
        old_mouse_in_window= mouse_in_window;

        while (XPending(config.xdpy))
        {
            XNextEvent(config.xdpy, &xe);
            switch (xe.type)
            {
            case FocusIn:
                mouse_in_window= 1;
                break;
            case FocusOut:
                mouse_in_window= 0;
                break;
            /* TODO: redraw window if requirement implied? */
            }
        }

        switch (curr_anim_state)
        {
        case ANIM_STOP:
            /* oneko:
             * switches immediately to "awake" state if moving;
             * maintains current animation briefly otherwise
             * on timeout, moves to pawing (togi) or rests (jare)
             */
            if (mouse_in_window != old_mouse_in_window)
                config.anim_state_count= 0;
            else if ( (config.anim_state_count > 16)   /* ~2 secs */
                    && !mouse_in_window)
                curr_anim_state= ANIM_JARE;
            else
                draw(1);
            break;
        case ANIM_JARE:
            /* oneko:
             * switches immediately to "awake" state if moving
             * maintains animation briefly otherwise
             * moves to "kaki" state
             */
            if (mouse_in_window)
                curr_anim_state= ANIM_STOP;
            else if (config.anim_state_count > 16)   /* ~2 secs */
                curr_anim_state= ANIM_SLEEP;
            else
                draw(2);
            break;
        case ANIM_SLEEP:
            if (mouse_in_window)
                curr_anim_state= ANIM_STOP;
#if 0   /* sleeps indefinitely without this */
            else if (config.anim_state_count > 16)   /* ~2 secs */
                curr_anim_state= ANIM_STATE_LAST;
#else
            else if (config.anim_state_count >= MAX_TICK)
                config.anim_state_count-= MAX_TICK;
#endif
            if ( config.anim_state_count % 2 == 0)
                draw(3);
            else
                draw(4);
            break;
        default:
            curr_anim_state= ANIM_STOP;
        }

        if (curr_anim_state != old_anim_state)
        {
            config.anim_state_count= 0;
        }
        else
        {
            /* retain the frame shown until alarm or other signal */
            timerclear(&itimerval.it_interval);
            timerclear(&itimerval.it_value);

            itimerval.it_interval.tv_usec= interval_time;
            itimerval.it_value.tv_usec= interval_time;

            signal(SIGALRM, sighandler_alrm);
            signal(SIGUSR1, sighandler_usr);
            signal(SIGUSR2, sighandler_usr);
            setitimer(ITIMER_REAL, &itimerval, 0);

            pause();    /* await next signal */
        }
    }
    while (curr_anim_state != ANIM_STATE_LAST);
}

int main(const int argc, const char **argv)
{
    int ix;

    config.progname= argv[0];

    if (gui_init() != EXIT_SUCCESS)
    {
        printf("%s: initialisation failed\n", config.progname);
        return EXIT_FAILURE;
    }

    if (argc > 1)
    {
        int found= 0;

        for (ix= 0; bitmap_table[ix].name != NULL; ix++)
        {
            if (strcmp(argv[1], bitmap_table[ix].name) == 0)
            {
                config.character= ix;
                found= 1;
            }
        }
        if (!found)
        {
            fprintf(stderr, "%s: Character bitmap '%s' not found\n",
                    config.progname, argv[1]);
            exit(EXIT_FAILURE);
        }
    }

    run_animation();

    return gui_fini();
}
