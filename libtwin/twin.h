/*
 * TWIN.H --Definitions for Text WINdows.
 *
 */
#ifndef TWIN_H
#define TWIN_H

#include <unistd.h>
#include <stdint.h>
#include <apex.h>

#ifdef __cplusplus
extern "C"
{
#endif                                 /* C++ */

#define TWIN_DEFAULT_COLOUR 9
    typedef enum TwinCellAttribute_t
    {
        TwinNormal = 0x00,             /* a value, not a bit */
        TwinBold = 0x01,
        TwinDim = 0x02,
        TwinItalic = 0x04,
        TwinUnderline = 0x08,
        TwinFlashing = 0x10,
        TwinUnknown = 0x20,
        TwinReverse = 0x40,
        TwinAlt = 0x80,
    } TwinCellAttribute;

    typedef enum TwinState_t
    {
        TwinRegiond = 0x01,
        TwinVisible = 0x02
    } TwinState;

    typedef enum TwinEvent_t
    {                                  /* TODO: input handling */
        twin_tick,
        twin_move,
        twin_resize,
        twin_draw,
        twin_delete,
        twin_key_press,
        twin_mouse
    } TwinEvent;

    typedef struct TwinCell_t
    {                                  /* REVISIT: force alignment for int! */
        uint8_t fg, bg;                /* colours */
        uint8_t attr;                  /* TwinCellAttribute */
        uint8_t ch;                    /* codepoint, or line-graphics bits */
    } TwinCell;

    typedef struct TwinCoordinate_t
    {
        int row, column;
    } TwinCoordinate;

    typedef struct TwinGeometry_t
    {
        TwinCoordinate position, size;
    } TwinGeometry;

    typedef struct TwinRegion_t
    {
        TwinCoordinate min, max;
    } TwinRegion;


    struct Twindow_t;
    typedef int (*TwinProc)(struct Twindow_t * twin, TwinEvent event,
                            void *arg);
    typedef struct Twindow_t
    {
        TwinGeometry geometry;
        TwinRegion damage;             /* if .state & TwinDamage */
        TwinCoordinate cursor;
        TwinCell style;
        int state;                     /* TwinState */
        TwinCell *frame;               /* base: array of cells */
        struct Twindow_t *parent;
        struct Twindow_t *child;
        struct Twindow_t *sibling;
    } Twindow;

    inline int twin_cell(TwinGeometry geometry, int row, int column)
    {
        return row * geometry.size.column + column;
    }

    Twindow *twin_alloc(void);
    void free_twin(Twindow * twin);

    Twindow *twin_init(Twindow * twin, Twindow * parent,
                       int row, int column, int height, int width,
                       TwinCell * frame);

#define new_twin(parent, row, column, n_rows, n_columns, frame) \
    twin_init(twin_alloc(), parent, row, column, n_rows, n_columns, frame)
#define init_twin(twin, parent, row, column, frame)                \
        twin_init(twin, parent, rows, columns, NEL(frame), NEL(frame[0]), frame)

    void twin_reset(Twindow * twin);
    Twindow *twin_cursor(Twindow * twin, int row, int column);
    Twindow *twin_attr(Twindow * twin, TwinCell attr);
    int twin_set_cell(Twindow * twin, int row, int col, TwinCell cell);
    Twindow *twin_puts(Twindow * twin, const char *text);
    Twindow *twin_printf(Twindow * twin, const char *format,
                         ...) PRINTF_ATTRIBUTE(2, 3);
    Twindow *twin_clear(Twindow * twin);
    Twindow *twin_box(Twindow * tw, int row, int column,
                      int n_rows, int n_columns);
    Twindow *twin_hline(Twindow * twin, int row, int column, int size);
    Twindow *twin_vline(Twindow * twin, int row, int column, int size);
    Twindow *twin_compose(Twindow * dst, Twindow * src,
                          TwinCoordinate offset);
    Twindow *twin_add_child(Twindow * parent, Twindow * child);
    Twindow *twin_remove_child(Twindow * parent, Twindow * child);
#ifdef __cplusplus
}
#endif /* C++ */
#endif /* TWIN_H */
