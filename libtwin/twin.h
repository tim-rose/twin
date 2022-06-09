/*
 * TWIN.H --Definitions for Text WINdows.
 *
 */
#ifndef TWIN_H
#define TWIN_H

#include <unistd.h>
#include <xtd.h>

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
        TwinDamaged = 0x01,
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
    } TwinCell, *TwinCellPtr;

    typedef struct TwinCoordinate_t
    {
        int row, column;
    } TwinCoordinate, *TwinCoordinatePtr;

    typedef struct TwinGeometry_t
    {
        TwinCoordinate position, size;
    } TwinGeometry, *TwinGeometryPtr;

    typedef struct TwinDamage_t
    {
        TwinCoordinate min, max;
    } TwinDamage, *TwinDamagePtr;

    struct Twindow;

    typedef int (*TwinProc)(struct Twindow * twin, TwinEvent event,
                            void *arg);

    typedef struct Twindow_t
    {
        const char *name;
        TwinGeometry geometry;
        TwinDamage damage;             /* if .state & TwinDamage */
        TwinCoordinate cursor;
        TwinCell style;
        int state;                     /* TwinState */
        TwinCellPtr frame;             /* base: array of cells */
        struct Twindow_t *parent;
        struct Twindow *child;
        struct Twindow *sibling;
    } Twindow, *TwindowPtr;

    inline int twin_cell(TwinGeometry geometry, int row, int column)
    {
        return row * geometry.size.column + column;
    }

    TwindowPtr twin_alloc(void);
    void free_twin(TwindowPtr twin);

    TwindowPtr twin_init(TwindowPtr twin, const char *name, TwindowPtr parent,
                         int row, int column, int height, int width,
                         TwinCellPtr frame);

#define new_twin(name, parent, row, column, n_rows, n_columns, frame) \
    twin_init(twin_alloc(), name, parent, row, column, n_rows, n_columns, frame)
#define init_twin(twin, name, parent, row, column, frame)                \
        twin_init(twin, name, parent, rows, columns, NEL(frame), NEL(frame[0]), frame)

    void twin_reset(TwindowPtr twin);
    TwindowPtr twin_cursor(TwindowPtr twin, int row, int column);
    TwindowPtr twin_attr(TwindowPtr twin, TwinCell attr);
    int twin_set_cell(TwindowPtr twin, int row, int col, TwinCell cell);
    TwindowPtr twin_puts(TwindowPtr twin, const char *text);
    TwindowPtr twin_printf(TwindowPtr twin, const char *format,
                           ...) PRINTF_ATTRIBUTE(2, 3);
    TwindowPtr twin_clear(TwindowPtr twin);
    TwindowPtr twin_box(TwindowPtr tw, int row, int column,
                        int n_rows, int n_columns);
    TwindowPtr twin_hline(TwindowPtr twin, int row, int column, int size);
    TwindowPtr twin_vline(TwindowPtr twin, int row, int column, int size);
    TwindowPtr twin_compose(TwindowPtr dst, TwindowPtr src,
                            TwinCoordinate offset);
#ifdef __cplusplus
}
#endif /* C++ */
#endif /* TWIN_H */
