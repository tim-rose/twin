/*
 * XTERMINATOR.C --Control for xterm-like devices.
 *
 * Contents:
 * xterminator_init()  --Initialise the Xterminator structure.
 * close_xterminator() --Close, release resources, reset terminal.
 * xt_sync()           --Render any changes to the device.
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
#include <xterminator.h>
#include <log.h>
#include <estring.h>

#ifdef DEBUG_TTY
#define SO "<so>"
#define SI "<si>"
#define ESC "<esc>"
#else
#define SO "\016"
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


static int xt_style(XterminatorPtr xt, TwinCell style);
static void xt_cursor(XterminatorPtr xt, int row, int column);

XterminatorPtr new_xterminator(int input, FILE * output)
{
    XterminatorPtr xt =
        xterminator_init(malloc(sizeof(Xterminator)), input, output);

    return xt;
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
XterminatorPtr xterminator_init(XterminatorPtr xt, int input, FILE * output)
{
    struct winsize size;

    if (ioctl(fileno(output), TIOCGWINSZ, &size) < 0)
    {
        log_sys(LOG_ERR, "cannot get window size");
    }
    debug("%s(): size: %d rows, %d cols", __func__, size.ws_row, size.ws_col);

    memset(xt, 0, sizeof(*xt));
    xt->input = input;
    xt->output = output;
    setvbuf(xt->output, NULL, _IOFBF, 0);

    twin_init(&xt->root,
              "root", NULL, 0, 0, size.ws_row, size.ws_col,
              malloc(size.ws_row * size.ws_col * sizeof(TwinCell)));
    twin_init(&xt->screen,
              "screen", NULL, 0, 0, size.ws_row, size.ws_col,
              malloc(size.ws_row * size.ws_col * sizeof(TwinCell)));
    return xt;                         /* success */
}

void open_xterminator(XterminatorPtr xt)
{
    fputs(xt_init_cmd, xt->output);
    fflush(xt->output);
}


/*
 * close_xterminator() --Close, release resources, reset terminal.
 *
 * Remarks:
 * This routine outputs some cleanup capabilities to the terminal
 * device.
 */
void close_xterminator(XterminatorPtr xt)
{
    TwinCell no_style = { 0 };

    xt_style(xt, no_style);
    fputs(xt_end_cmd, xt->output);
}


/*
 * xt_sync() --Render any changes to the device.
 *
 * Returns: (int)
 * The number of changes.
 */
int xt_sync(XterminatorPtr xt)
{
    int change = 0;


    debug("%s(): position: %d, %d. size: %d, %d",
          __func__,
          xt->root.geometry.position.row, xt->root.geometry.position.column,
          xt->root.geometry.size.row, xt->root.geometry.size.column);
    debug("%s(): damage: min: %d, %d. max: %d, %d",
          __func__,
          xt->root.damage.min.row, xt->root.damage.min.column,
          xt->root.damage.max.row, xt->root.damage.max.column);

    if (!(xt->root.state & TwinDamaged))
    {
        return change;                 /* nothing is damaged */
    }

    for (int r = xt->root.damage.min.row; r <= xt->root.damage.max.row; ++r)
    {
        for (int c = xt->root.damage.min.column;
             c <= xt->root.damage.max.column; ++c)
        {                              /* TODO: optimise for trailing space? */
            int offset = twin_cell(xt->root.geometry, r, c);
            TwinCell cell = xt->root.frame[offset];

            if (memcmp(&cell, &xt->screen.frame[offset], sizeof(TwinCell)) ==
                0)
            {
                continue;
            }
            xt_cursor(xt, r, c);
            xt_style(xt, cell);

            if ((cell.attr & TwinAlt) && cell.ch < 16)
            {                          /* line graphic */
                fputc(xt_line_map[cell.ch], xt->output);
            }
            else
            {
                fputc(cell.ch, xt->output);
            }
            /* note: raw assignment avoids twin_set_cell()'s damage control */
            xt->screen.frame[offset] = xt->root.frame[offset];
            xt->screen.cursor.column += 1;
            ++change;
#ifdef DEBUG_TTY
            fputc('\n', xt->output);
#endif /* DEBUG_TTY */
        }
    }
    fflush(xt->output);
    twin_reset(&xt->root);
    debug("%s(): %d changes", __func__, change);
    return change;
}

static int xt_style(XterminatorPtr xt, TwinCell style)
{
    int change = 0;
    TwinCell screen_style = xt->screen.style;

    xt->screen.style = style;
    if (screen_style.attr != style.attr)
    {                                  /* adjust attributes */
        if ((screen_style.attr & TwinAlt) != (style.attr & TwinAlt))
        {                              /* handle alt. character set */
            fputs((style.attr & TwinAlt) ? SO : SI, xt->output);
            change = 1;
        }
        screen_style.attr &= ~TwinAlt; /* clear ACS differences */
        style.attr &= ~TwinAlt;

        if (screen_style.attr != style.attr)
        {                              /* handle misc. other attributes */
            fputs(ESC "[", xt->output);
            for (int i = 0; i < 8; ++i)
            {
                if (style.attr & (1 << i))
                {                      /* note: first ';' clears attributes */
                    fprintf(xt->output, ";%d", i + 1);
                }
            }
            fputc('m', xt->output);
            screen_style.fg = TWIN_DEFAULT_COLOUR;  /* mode changes reset fg, bg */
            screen_style.bg = TWIN_DEFAULT_COLOUR;
            change = 1;
        }
    }

    if (screen_style.fg != style.fg)
    {                                  /* adjust foreground colour */
        if (style.fg <= 9)
        {
            fprintf(xt->output, xt_fg_8_cmd, style.fg);
        }
        else
        {
            fprintf(xt->output, xt_fg_256_cmd, style.fg);
        }
        change = 1;
    }
    if (screen_style.bg != style.bg)
    {                                  /* adjust background too */
        if (style.bg <= 9)
        {
            fprintf(xt->output, xt_bg_8_cmd, style.bg);
        }
        else
        {
            fprintf(xt->output, xt_bg_256_cmd, style.bg);
        }
        change = 1;
    }
    return change;
}

static void xt_cursor(XterminatorPtr xt, int row, int column)
{
    if (xt->screen.cursor.row == row && xt->screen.cursor.column == column)
    {
        return;                        /* we're already there */
    }
    /* TODO: logic to move cursor efficiently on same row */
    fprintf(xt->output, xt_cup_cmd, row + 1, column + 1);
    xt->screen.cursor.row = row;
    xt->screen.cursor.column = column;
}




/*
 * free_xterminator() --Release any resources used by a Xterminator.
 */
void free_xterminator(XterminatorPtr xt)
{
    if (xt->screen.frame != NULL)
    {
        free(xt->screen.frame);
    }
    if (xt->root.frame != NULL)
    {
        free(xt->root.frame);
    }
    memset(xt, 0, sizeof(*xt));        /* safety: clear bytes */
    free(xt);
}

void xt_mainloop(XterminatorPtr xt)
{
    static const TwinCoordinate no_offset = { 0, 0 };

    for (;;)
    {
        twin_compose(&xt->root, &xt->root, no_offset);
        xt_sync(xt);
        /* wait for input or timer... */
        /* and then do what!? */
    }
}
