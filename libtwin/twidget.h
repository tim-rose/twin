/*
 * TWIDGET.H --Text widgets, windows with custom behaviour.
 *
 * Remarks:
 * This is an MVC experiment...
 *
 */
#ifndef TWIDGET_H
#define TWIDGET_H

#include <twin.h>

#ifdef __cplusplus
extern "C"
{
#endif                                 /* C++ */
    typedef struct Twidget_t Twidget;

    struct Twidget_t
    {
        Twindow window;
        const char *name;
        TwinProc control;       /* update the model based on an event */
        TwinProc draw;                  /* render the model into the window */
    };

    typedef struct TwidgetText_t
    {
        Twidget widget;
        const char *text;
    } TwidgetText;

    Twidget *init_twidget(Twidget * widget, const char *name,
                          Twindow * parent, TwinGeometry * geometry,
                          TwinProc control);
    TwidgetText *init_twidget_text(TwidgetText * widget, const char *name,
                                   Twindow * parent, TwinGeometry * geometry,
                                   const char *text);
#ifdef __cplusplus
}
#endif                                 /* C++ */
#endif                                 /* TWIDGET_H */
