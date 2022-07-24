/*
 * TWIDGET.C --Routines for Text Widgets.
 *
 * Contents:
 *
 * Remarks:
 */
#include <string.h>
#include <log.h>
#include "twidget.h"


Twidget *init_twidget(Twidget * widget, const char *name, Twindow * parent,
                      TwinGeometry * geometry, TwinProc control)
{
    Twindow *twin = &widget->window;

    memset(widget, 0, sizeof(*widget));

    if (parent != NULL)
    {
        widget->name = name;
        twin->parent = parent;
        twin_add_child(parent, twin);
        twin->geometry = *geometry;
        widget->control = control;
        return widget;                 /* success: return initialised  */
    }
    return NULL;                       /* failure: no parent */
}

static int twin_text_control(Twindow * twin, TwinEvent event, void *arg)
{
    return 0;
}


TwidgetText *init_twidget_text(TwidgetText * widget, const char *name,
                               Twindow * parent, TwinGeometry * geometry,
                               const char *text)
{
    widget = (TwidgetText *) init_twidget((Twidget *) widget, name, parent,
                                          geometry, twin_text_control);
    if (widget != NULL)
    {
        widget->text = text;
    }
    return widget;
}
