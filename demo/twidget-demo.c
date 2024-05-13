/*
 * TWIDGET-DEMO.C --Demo program for text widgets.
 */
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <apex.h>
#include <apex/log.h>
#include <xterminator.h>
#include <twidget.h>

static int resize_signal;
static Xterminator xterm;
static TwidgetText poem;
static TwinGeometry poem_geometry = { {10, 20}, {4, 20} };

const char poem_text[] =
    "The boy stood on the burning deck,\nwith a pocket full of crackers";


static void xterminate(void)
{
    close_xterminator(&xterm);
    free_xterminator(&xterm);
}


static void exit_gracefully(int UNUSED(signal))
{
    xterminate();
    exit(0);
}


static void ack_resize(int signal)
{
    resize_signal = 1;
}


static void resize_root(Xterminator * xt)
{
    struct winsize size;

    ioctl(xt->input, TIOCGWINSZ, &size);

    /* root.geometry.size.row = size.ws_row; */
    /* root.geometry.size.column = size.ws_col; */
    /* TODO: resize root structure! */
}


int main(int argc, char *argv[])
{
    FILE *output = stdout;

    if (argc > 1)
    {
        if ((output = fopen(argv[1], "w")) == NULL)
        {
            log_sys_quit(1, "cannot open \"%s\"", argv[1]);
        }
    }

    srand(0);
    signal(SIGWINCH, ack_resize);
    signal(SIGINT, exit_gracefully);
    signal(SIGQUIT, exit_gracefully);

    if (xterminator_init(&xterm, STDIN_FILENO, output) == NULL)
    {
        log_sys_quit(1, "cannot initialise terminal \"%s\"", argv[1]);
    }
    open_xterminator(&xterm);
    init_twidget_text(&poem, "poem", &xterm.root, &poem_geometry, poem_text);

    xterm_sync(&xterm);
    sleep(500);
    exit_gracefully(0);
}
