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
        Twindow *focus;
    } Xterminator;

    Xterminator *new_xterminator(int input, FILE * output);
    Xterminator *xterminator_init(Xterminator * xt, int input, FILE * output);
    void free_xterminator(Xterminator * xt);

    void open_xterminator(Xterminator * xt);
    void close_xterminator(Xterminator * xt);

    void resize_xterminator(Xterminator * xt);
    TwinCell xt_cell(Xterminator * xt, int row, int col, TwinCell cell);
    int xt_sync(Xterminator * xt);
    int xt_clear(Xterminator * xt);
#ifdef __cplusplus
}
#endif                                 /* C++ */
#endif                                 /* XTERMINATOR_H */
