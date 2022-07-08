/*
 * TWIN.C --Routines for Twin, the Text Windows library.
 *
 * Contents:
 *
 * Remarks:
 */
#include <log.h>
#include <estring.h>
#include "twin.h"

extern inline int twin_cell(TwinGeometry geometry, int row, int column);

static inline int twin_cmp_style(TwinCell a, TwinCell b)
{
    return !(a.fg == b.fg && a.bg == b.bg && a.attr == b.attr);
}

static TwinCell blank = {
    TWIN_DEFAULT_COLOUR, TWIN_DEFAULT_COLOUR, TwinNormal, ' '
};

Twindow *twin_alloc(void)
{
    return malloc(sizeof(Twindow));
}


Twindow *twin_init(Twindow * twin, Twindow * parent,
                   int row, int column, int height, int width,
                   TwinCell * frame)
{
    memset(twin, 0, sizeof(*twin));    /* nulls linkage pointers */
    twin->geometry.position.row = row;
    twin->geometry.position.column = column;
    twin->geometry.size.row = height;
    twin->geometry.size.column = width;
    twin->frame = frame;
    twin->style = blank;
    twin_reset(twin);                  /* not damaged */
    twin_clear(twin);                  /* all spaces */
    return twin;
}


void free_twin(Twindow * twin)
{
    if (twin->frame)
    {
        free(twin->frame);
    }
    free(twin);
}


void twin_reset(Twindow * twin)
{
    twin->damage.min.row = twin->geometry.size.row;
    twin->damage.min.column = twin->geometry.size.column;
    twin->damage.max.row = 0;
    twin->damage.max.column = 0;
    twin->state &= ~TwinRegiond;
}


Twindow *twin_cursor(Twindow * twin, int row, int column)
{
    twin->cursor.row = row;
    twin->cursor.column = column;
    return twin;
}


int twin_set_cell(Twindow * twin, int row, int col, TwinCell cell)
{
    if (row < 0 || row > twin->geometry.size.row
        || col < 0 || col > twin->geometry.size.column)
    {
        return 0;                      /* failure: bounds check */
    }

    int offset = twin_cell(twin->geometry, row, col);

    if (memcmp(&cell, &twin->frame[offset], sizeof(TwinCell)) != 0)
    {                                  /* update damage */
        if (row < twin->damage.min.row)
        {
            twin->damage.min.row = row;
        }
        if (col < twin->damage.min.column)
        {
            twin->damage.min.column = col;
        }
        if (row > twin->damage.max.row)
        {
            twin->damage.max.row = row;
        }
        if (col > twin->damage.max.column)
        {
            twin->damage.max.column = col;
        }
        twin->state |= TwinRegiond;
        twin->frame[offset] = cell;
    }
    return 1;                          /* success */
}


Twindow *twin_puts(Twindow * twin, const char *text)
{
    int col;
    TwinCell cell = twin->style;

    for (col = twin->cursor.column; *text != '\0'; ++text, ++col)
    {
        if (col >= twin->geometry.size.column)
        {
            break;                     /* overflow */
        }
        cell.ch = (uint8_t) * text;
        twin_set_cell(twin, twin->cursor.row, col, cell);
    }
    twin->cursor.column = col;
    return twin;
}

Twindow *twin_hline(Twindow * twin, int row, int column, int size)
{
    int c;
    int start = column;
    int end = start + size;
    static const char line_frag[] = { 0x02, 0x0a, 0x08 };

    debug("%s(%d, %d): size:%d", __func__, row, column, size);

    if (size < 0)
    {                                  /* flip -ve size */
        start = column + size + 1;     /* -ve! */
        end = column + 1;
    }
    if (row + size < 0 || row >= twin->geometry.size.row)
    {                                  /* out of range */
        return twin;
    }

    for (c = start; c < end; ++c)
    {
        int offset = twin_cell(twin->geometry, row, c);
        TwinCell cell = twin->frame[offset];
        char ch = '\0';
        TwinCell new_cell = twin->style;

        if (c < 0)
        {
            continue;                  /* underflow */
        }
        if (c >= twin->geometry.size.column)
        {
            break;                     /* overflow */
        }
        new_cell.attr |= TwinAlt;
        if (c == column)
        {                              /* true start */
            ch = line_frag[0];         /* " -" */
        }
        else if (c == end - 1)
        {                              /* true end */
            ch = line_frag[2];         /* "- " */
        }
        else
        {
            ch = line_frag[1];         /* "--" */
        }

        if ((cell.attr & TwinAlt) && (cell.ch < 16)
            && twin_cmp_style(cell, new_cell) == 0)
        {
            new_cell.ch = (uint8_t) (cell.ch | ch); /* merge line graphics */
        }
        else
        {
            new_cell.ch = (uint8_t) ch; /* replace with line graphic */
        }
        twin_set_cell(twin, row, c, new_cell);
    }
    return twin;
}


Twindow *twin_vline(Twindow * twin, int row, int column, int size)
{
    int r;
    int start = row;
    int end = start + size;
    static const char line_frag[] = { 0x04, 0x05, 0x01 };

    debug("%s(%d, %d): size:%d", __func__, row, column, size);

    if (size < 0)
    {                                  /* flip -ve size */
        start = row + size + 1;        /* -ve! */
        end = row + 1;
    }

    if (column + size < 0 || column >= twin->geometry.size.column)
    {                                  /* out of range */
        return twin;
    }

    for (r = start; r < end; ++r)
    {
        int offset = twin_cell(twin->geometry, r, column);
        TwinCell cell = twin->frame[offset];
        char ch = '\0';
        TwinCell new_cell = twin->style;

        if (r < 0)
        {
            continue;                  /* underflow */
        }
        if (r >= twin->geometry.size.row)
        {
            break;                     /* overflow */
        }
        new_cell.attr |= TwinAlt;
        if (r == row)
        {                              /* true start */
            ch = line_frag[0];
        }
        else if (r == end - 1)
        {                              /* true end */
            ch = line_frag[2];
        }
        else
        {
            ch = line_frag[1];
        }

        if ((cell.attr & TwinAlt) && (cell.ch < 16)
            && twin_cmp_style(cell, new_cell) == 0)
        {
            new_cell.ch = (uint8_t) (cell.ch | ch); /* merge line graphics */
        }
        else
        {
            new_cell.ch = (uint8_t) ch; /* replace with line graphic */
        }
        debug("%s(@%d): 0x%hhx+0x%hhx -> 0x%hhx",
              __func__, r, cell.ch, ch, new_cell.ch);

        twin_set_cell(twin, r, column, new_cell);
    }
    return twin;
}

Twindow *twin_box(Twindow * tw, int row, int column,
                  int n_rows, int n_columns)
{
    twin_hline(tw, row, column, n_columns);
    twin_vline(tw, row, column + n_columns - 1, n_rows);
    twin_hline(tw, row + n_rows - 1, column, n_columns);
    twin_vline(tw, row, column, n_rows);
    return tw;
}


Twindow *twin_printf(Twindow * twin, const char *format, ...)
{
    char text[twin->geometry.size.column];

    err("%s() is not implemented", __func__);
    return NULL;
}

Twindow *twin_clear(Twindow * twin)
{
    TwinCell *cell = twin->frame;

    for (int i = 0;
         i < twin->geometry.size.row * twin->geometry.size.column; ++i)
    {
        *cell++ = blank;
    }
    return twin;
}

Twindow *twin_compose(Twindow * dst, Twindow * src, TwinCoordinate offset)
{
    if ((src->state & TwinRegiond) && src != dst)   /* catch tx->root */
    {
        for (int r = src->damage.min.row; r <= src->damage.max.row; ++r)
        {
            for (int c = src->damage.min.column; c <= src->damage.max.column;
                 ++c)
            {                          /* overwrite damaged/changed cells */
                twin_set_cell(dst,
                              r + src->geometry.position.row + offset.row,
                              c + src->geometry.position.column +
                              offset.column,
                              src->frame[twin_cell(src->geometry, r, c)]);
            }
        }
    }
    /* adjust offset for children... */
    offset.row += src->geometry.position.row;
    offset.column += src->geometry.position.column;

    for (Twindow * child = src->child; child != NULL; child = child->sibling)
    {                                  /* recursively compose children */
        twin_compose(dst, child, offset);
    }
    return dst;
}
