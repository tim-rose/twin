
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <xtd.h>
#include <log.h>
#include <xterminator.h>

static int resize_signal;
static XterminatorPtr xterminator;

static void style_box(TwindowPtr tw, int row, int column);
static void sampler_box(TwindowPtr tw, int row, int column);
static void colour_box(TwindowPtr tw, int row, int column);
static void clip_boxes(TwindowPtr tw);
static void boxes(TwindowPtr tw);

static void xterminate(void)
{
    if (xterminator != NULL)
    {
        close_xterminator(xterminator);
        free_xterminator(xterminator);
        xterminator = NULL;
    }
}


static void exit_gracefully(int signal)
{
    xterminate();
    exit(0);
}


static void ack_resize(int signal)
{
    resize_signal = 1;
}


static void resize_root(XterminatorPtr xt)
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
    atexit(exit_gracefully);

    xterminator = new_xterminator(0, output);
    TwindowPtr root = &xterminator->root;

    open_xterminator(xterminator);

    twin_box(root, 0, 0, root->geometry.size.row, root->geometry.size.column);
    twin_cursor(root, 0, 1);
    twin_puts(root, "twin-root");
    xt_sync(xterminator);

    style_box(root, 3, 5);
    sampler_box(root, 15, 2);
    colour_box(root, 3, 30);
    boxes(root);
    sleep(500);
    close_xterminator(xterminator);
}


static void style_box(TwindowPtr tw, int row, int column)
{
    tw->style.fg = 2;
    twin_box(tw, row, column, 9, 12);
    tw->style.fg = TWIN_DEFAULT_COLOUR;
    tw->style.attr = TwinNormal;
    twin_cursor(tw, row, ++column);
    twin_puts(tw, "styles");
    xt_sync(xterminator);
    usleep(500000);

    twin_cursor(tw, ++row, column);
    twin_puts(tw, "Normal");
    tw->style.attr = TwinBold;
    twin_cursor(tw, ++row, column);
    twin_puts(tw, "Bold");
    tw->style.attr = TwinDim;
    twin_cursor(tw, ++row, column);
    twin_puts(tw, "Dim");
    tw->style.attr = TwinItalic;
    twin_cursor(tw, ++row, column);
    twin_puts(tw, "Italic");
    tw->style.attr = TwinUnderline;
    twin_cursor(tw, ++row, column);
    twin_puts(tw, "Underline");
    tw->style.attr = TwinFlashing;
    twin_cursor(tw, ++row, column);
    twin_puts(tw, "Flash");
    tw->style.attr = TwinReverse;
    twin_cursor(tw, ++row, column);
    twin_puts(tw, "Reverse");
    xt_sync(xterminator);
    tw->style.attr = TwinNormal;
    sleep(1);
}


static void sampler_box(TwindowPtr tw, int row, int column)
{
    tw->style.fg = 3;
    twin_box(tw, row, column, 4, 63);
    tw->style.fg = TWIN_DEFAULT_COLOUR;
    twin_cursor(tw, row++, ++column);
    twin_puts(tw, "sampler");
    xt_sync(xterminator);
    usleep(500000);
    for (int i = 0; i < 32; ++i)
    {
        char str[2] = { '`' + i, '\0' };

        twin_cursor(tw, row, column + i * 2);
        twin_puts(tw, str);
    }
    xt_sync(xterminator);
    usleep(500000);
    ++row;
    tw->style.attr = TwinAlt;
    for (int i = 0; i < 32; ++i)
    {
        char str[2] = { '`' + i, '\0' };

        twin_cursor(tw, row, column + i * 2);
        twin_puts(tw, str);
        xt_sync(xterminator);
        usleep(50000);
    }
    tw->style.attr = TwinNormal;
    xt_sync(xterminator);
    sleep(1);
}


static void colour_box(TwindowPtr tw, int row, int column)
{
    tw->style.fg = 6;
    twin_box(tw, row, column, 8, 14);
    tw->style.fg = TWIN_DEFAULT_COLOUR;
    twin_cursor(tw, row++, ++column);
    twin_puts(tw, "colour");
    xt_sync(xterminator);
    usleep(500000);

    for (int i = 16; i < 255; i += 36)
    {
        for (int r = 0; r < 6; ++r)
        {
            twin_cursor(tw, row + r, column);
            for (int c = 0; c < 6; ++c)
            {
                tw->style.bg = i + 6 * r + c;
                twin_puts(tw, "  ");
            }
        }
        xt_sync(xterminator);
        usleep(500000);
    }
    tw->style.bg = TWIN_DEFAULT_COLOUR;
    xt_sync(xterminator);
    sleep(1);
}


static void clip_boxes(TwindowPtr tw)
{
    twin_box(tw, -1, -1, 3, 3);
    usleep(500000);
    xt_sync(xterminator);

    twin_box(tw, -1, tw->geometry.size.column / 2, 3, 3);
    usleep(500000);
    xt_sync(xterminator);

    twin_box(tw, -1, tw->geometry.size.column - 2, 3, 3);
    usleep(500000);
    xt_sync(xterminator);

    twin_box(tw, tw->geometry.size.row / 2, -1, 3, 3);
    usleep(500000);
    xt_sync(xterminator);

    twin_box(tw, tw->geometry.size.row / 2, tw->geometry.size.column - 2, 3,
             3);
    usleep(500000);
    xt_sync(xterminator);

    twin_box(tw, tw->geometry.size.row - 2, -1, 3, 3);
    usleep(500000);
    xt_sync(xterminator);

    twin_box(tw, tw->geometry.size.row - 2, tw->geometry.size.column / 2, 3,
             3);
    usleep(500000);
    xt_sync(xterminator);

    twin_box(tw, tw->geometry.size.row - 2, tw->geometry.size.column - 2, 3,
             3);
    usleep(500000);
    xt_sync(xterminator);
    sleep(1);
}


static void boxes(TwindowPtr tw)
{
    clip_boxes(tw);
    for (int i = 0; i < 300; ++i)
    {
        twin_box(tw,
                 rand() % (tw->geometry.size.row - 3),
                 rand() % (tw->geometry.size.column - 8),
                 rand() % 4 + 2, rand() % 8 + 2);
        xt_sync(xterminator);
        usleep(50000);
    }
}
