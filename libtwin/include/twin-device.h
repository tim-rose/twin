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
    typedef struct TwinRegion_t
    {
        int min, max;                   /* region of columns damaged */
    } TwinRegion, *TwinDamagePtr;

    typedef struct Xterminator_t
    {
        int input;
        FILE *output;
        TwinCell* screen;            /* frame */
        Twindow* root;
        TwinRegion* damage;
        Twindow* focus;
    } Xterminator, *XterminatorPtr;


    Xterminator* new_xterminator(int input, FILE *output);
    void free_twin_dev(Xterminator* xt);

    Xterminator* xterminator_init(
        Xterminator* xt,
        int input, FILE *output,
        TwinCell* screen, TwindowPtr root, TwinRegionPtr damage);


    void close_xterminator(Xterminator* xt);

    void resize_xterminator(Xterminator* xt);
    Xterminator* sync_xterminator(XterminatorPtr xt);
#ifdef __cplusplus
}
#endif                                 /* C++ */
#endif                                 /* XTERMINATOR_H */
