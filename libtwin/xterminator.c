/*
 * XTERMINATOR.C --Control for xterm-like devices.
 *
 * Contents:
 * xterminator_init()  --Initialise the Xterminator structure.
 * close_xterminator() --Close, release resources, reset terminal.
 * xterm_sync()        --Render any changes to the device.
 * free_xterminator()  --Release any resources used by a Xterminator.
 *
 * Remarks:
 * The Xterminator struct models an xterm-compatible screen, and the
 * files connected to it.  There are two frame buffers:
 * * root --contains the screen changes, and its sub-windows
 * * screen --contains the current state of the actual screen.
 *
 * Root and screen are compared when updating the actual screen.
 */
#include <sys/ioctl.h>
#include <apex.h>
#include <apex/log.h>
#include <apex/estring.h>
#include "xterminator.h"

#ifdef DEBUG_TTY
#define SO "<so>"
#define SI "<si>"
#define ESC "<esc>"
#else
#define SO "\016"                      /* TODO: \e(0, \e(B */
#define SI "\017"
#define ESC "\033"
#endif /* DEBUG_TTY */

static const char xt_init_cmd[] =      /* initialisation commands... */
    ESC "[!p"                          /* soft reset */
    ESC "[?3;4l"                       /* normal-width, jump scroll */
    ESC "[4l"                          /* replace mode */
    ESC ">"                            /* "normal" keypad */
    ESC ")0"                           /* enable DEC alternate characters */
    ESC "[?1049h";                     /* save cursor, switch to alt. screen */
static const char xt_end_cmd[] = ESC "[?1049l"; /* restore cursor, screen */
static const char xt_cup_cmd[] = ESC "[%d;%dH";
static const char xt_clear_cmd[] = ESC "[2J";
static const char xt_ed_cmd[] = ESC "[J";   /* ...to end of screen */
static const char xt_line_map[] = "~xqmxxltqjqvkuwn";
static const char xt_fg_8_cmd[] = ESC "[3%dm";
static const char xt_fg_256_cmd[] = ESC "[38;5;%dm";
static const char xt_bg_8_cmd[] = ESC "[4%dm";
static const char xt_bg_256_cmd[] = ESC "[48;5;%dm";


static int xterm_style(Xterminator * xterm, TwinCell style);
static void xterm_cursor(Xterminator * xterm, int row, int column);

Xterminator *new_xterminator(int input, FILE * output)
{
    Xterminator *xterm =
        xterminator_init(malloc(sizeof(Xterminator)), input, output);

    return xterm;
}


/*
 * xterminator_init() --Initialise the Xterminator structure.
 *
 * Parameters:
 * xterminator  --the Xterminator to initialise.
 *
 * Returns: (XterminatorPtr)
 * Success: an initialised Xterminator; Failure: NULL.
 */
Xterminator *xterminator_init(Xterminator * xterm, int input, FILE * output)
{
    struct winsize size;

    if (ioctl(fileno(output), TIOCGWINSZ, &size) < 0)
    {
        log_sys(LOG_ERR, "cannot get window size");
    }
    debug("%s(): size: %d rows, %d cols", __func__, size.ws_row, size.ws_col);

    memset(xterm, 0, sizeof(*xterm));
    xterm->input = input;
    xterm->output = output;
    setvbuf(xterm->output, NULL, _IOFBF, 0);

    twin_init(&xterm->root,
              NULL, 0, 0, size.ws_row, size.ws_col,
              malloc(size.ws_row * size.ws_col * sizeof(TwinCell)));
    twin_init(&xterm->screen,
              NULL, 0, 0, size.ws_row, size.ws_col,
              malloc(size.ws_row * size.ws_col * sizeof(TwinCell)));
    return xterm;                      /* success */
}

void open_xterminator(Xterminator * xterm)
{
    fputs(xt_init_cmd, xterm->output);
    fflush(xterm->output);
}


/*
 * close_xterminator() --Close, release resources, reset terminal.
 *
 * Remarks:
 * This routine outputs some cleanup capabilities to the terminal
 * device.
 */
void close_xterminator(Xterminator * xterm)
{
    TwinCell no_style = { 0 };

    xterm_style(xterm, no_style);
    fputs(xt_end_cmd, xterm->output);
}


/*
 * xterm_sync() --Render any changes to the device.
 *
 * Returns: (int)
 * The number of changes.
 */
int xterm_sync(Xterminator * xterm)
{
    int change = 0;


    debug("%s(): position: %d, %d. size: %d, %d",
          __func__,
          xterm->root.geometry.position.row,
          xterm->root.geometry.position.column, xterm->root.geometry.size.row,
          xterm->root.geometry.size.column);
    debug("%s(): damage: min: %d, %d. max: %d, %d", __func__,
          xterm->root.damage.min.row, xterm->root.damage.min.column,
          xterm->root.damage.max.row, xterm->root.damage.max.column);

    if (!(xterm->root.state & TwinRegiond))
    {
        return change;                 /* nothing is damaged */
    }

    for (int r = xterm->root.damage.min.row; r <= xterm->root.damage.max.row;
         ++r)
    {
        for (int c = xterm->root.damage.min.column;
             c <= xterm->root.damage.max.column; ++c)
        {                              /* TODO: optimise for trailing space? */
            int offset = twin_cell(xterm->root.geometry, r, c);
            TwinCell cell = xterm->root.frame[offset];

            if (memcmp(&cell, &xterm->screen.frame[offset], sizeof(TwinCell))
                == 0)
            {
                continue;
            }
            xterm_cursor(xterm, r, c);
            xterm_style(xterm, cell);

            if ((cell.attr & TwinAlt) && cell.ch < 16)
            {                          /* line graphic */
                fputc(xt_line_map[cell.ch], xterm->output);
            }
            else
            {
                fputc(cell.ch, xterm->output);
            }
            /* note: raw assignment avoids twin_set_cell()'s damage control */
            xterm->screen.frame[offset] = xterm->root.frame[offset];
            xterm->screen.cursor.column += 1;
            ++change;
#ifdef DEBUG_TTY
            fputc('\n', xterm->output);
#endif /* DEBUG_TTY */
        }
    }
    fflush(xterm->output);
    twin_reset(&xterm->root);
    debug("%s(): %d changes", __func__, change);
    return change;
}

static int xterm_style(Xterminator * xterm, TwinCell style)
{
    int change = 0;
    TwinCell screen_style = xterm->screen.style;

    xterm->screen.style = style;
    if (screen_style.attr != style.attr)
    {                                  /* adjust attributes */
        if ((screen_style.attr & TwinAlt) != (style.attr & TwinAlt))
        {                              /* handle alt. character set */
            fputs((style.attr & TwinAlt) ? SO : SI, xterm->output);
            change = 1;
        }
        screen_style.attr &= ~TwinAlt; /* clear ACS differences */
        style.attr &= ~TwinAlt;

        if (screen_style.attr != style.attr)
        {                              /* handle misc. other attributes */
            fputs(ESC "[", xterm->output);
            for (int i = 0; i < 8; ++i)
            {
                if (style.attr & (1 << i))
                {                      /* note: first ';' clears attributes */
                    fprintf(xterm->output, ";%d", i + 1);
                }
            }
            fputc('m', xterm->output);
            screen_style.fg = TWIN_DEFAULT_COLOUR;  /* mode changes reset fg, bg */
            screen_style.bg = TWIN_DEFAULT_COLOUR;
            change = 1;
        }
    }

    if (screen_style.fg != style.fg)
    {                                  /* adjust foreground colour */
        if (style.fg <= 9)
        {
            fprintf(xterm->output, xt_fg_8_cmd, style.fg);
        }
        else
        {
            fprintf(xterm->output, xt_fg_256_cmd, style.fg);
        }
        change = 1;
    }
    if (screen_style.bg != style.bg)
    {                                  /* adjust background too */
        if (style.bg <= 9)
        {
            fprintf(xterm->output, xt_bg_8_cmd, style.bg);
        }
        else
        {
            fprintf(xterm->output, xt_bg_256_cmd, style.bg);
        }
        change = 1;
    }
    return change;
}

static void xterm_cursor(Xterminator * xterm, int row, int column)
{
    if (xterm->screen.cursor.row == row
        && xterm->screen.cursor.column == column)
    {
        return;                        /* we're already there */
    }
    /* TODO: logic to move cursor efficiently on same row */
    fprintf(xterm->output, xt_cup_cmd, row + 1, column + 1);
    xterm->screen.cursor.row = row;
    xterm->screen.cursor.column = column;
}




/*
 * free_xterminator() --Release any resources used by a Xterminator.
 */
void free_xterminator(Xterminator * xterm)
{
    if (xterm->screen.frame != NULL)
    {
        free(xterm->screen.frame);
    }
    if (xterm->root.frame != NULL)
    {
        free(xterm->root.frame);
    }
    memset(xterm, 0, sizeof(*xterm));  /* safety: clear bytes */
    free(xterm);
}

void xterm_mainloop(Xterminator * xterm)
{
    static const TwinCoordinate no_offset = { 0, 0 };

    for (;;)
    {
        twin_compose(&xterm->root, &xterm->root, no_offset);
        xterm_sync(xterm);
        /* wait for input or timer... */
        /* and then do what!? */
    }
}
