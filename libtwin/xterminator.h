/*
 * XTERMINATOR.H --Control for xterm-like devices.
 *
 */
#ifndef XTERMINATOR_H
#define XTERMINATOR_H

#include <stdio.h>
#include <twin.h>

#ifdef __cplusplus
extern "C"
{
#endif                                 /* C++ */
    /*
     * TermGraphic: --Names for the VT100 alternate character glyphs.
     */
    typedef enum TermGraphic_t
    {
        TermGraphDiamond = '`',
        TermGraphChecker,
        TermGraphTab,
        TermGraphFormfeed,
        TermGraphReturn,
        TermGraphLinefeed,
        TermGraphPlusminus,
        TermGraphNewline,
        TermGraphVerticaltab,
        TermGraph_tl,
        TermGraph_bl,
        TermGraph_rb,
        TermGraph_tr,
        TermGraph_trbl,
        TermGraph_scan4,
        TermGraph_scan3,
        TermGraph_scan2,
        TermGraph_scan1,
        TermGraph_scan0,
        TermGraph_trb,
        TermGraph_tbl,
        TermGraph_trl,
        TermGraph_rbl,
        TermGraph_tb,
        TermGraphLessequal,
        TermGraphGreaterequal,
        TermGraphPi,
        TermGraphNotequal,
        TermGraphPound,
        TermGraphDot
    } TermGraphic;


    typedef struct Xterminator_t
    {
        int input;
        FILE *output;
        Twindow screen;                /* frame */
        Twindow root;
        TwindowPtr focus;
    } Xterminator, *XterminatorPtr;

    XterminatorPtr new_xterminator(int input, FILE * output);
    XterminatorPtr xterminator_init(XterminatorPtr xt,
                                    int input, FILE * output);
    void free_xterminator(XterminatorPtr xt);

    void open_xterminator(XterminatorPtr xt);
    void close_xterminator(XterminatorPtr xt);

    void resize_xterminator(XterminatorPtr xt);
    TwinCell xt_cell(XterminatorPtr xt, int row, int col, TwinCell cell);
    int xt_sync(XterminatorPtr xt);
    int xt_clear(XterminatorPtr xt);
#ifdef __cplusplus
}
#endif                                 /* C++ */
#endif                                 /* XTERMINATOR_H */
