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
    typedef struct TwinDamage_t
    {
        int min, max;                   /* region of columns damaged */
    } TwinDamage, *TwinDamagePtr;

    typedef struct Xterminator_t
    {
        int input;
        FILE *output;
        TwinCellPtr screen;            /* frame */
        TwindowPtr root;
        TwinDamagePtr damage;
        TwindowPtr focus;
    } Xterminator, *XterminatorPtr;


    XterminatorPtr new_xterminator(int input, FILE *output);
    void free_twin_dev(XterminatorPtr xt);

    XterminatorPtr xterminator_init(
        XterminatorPtr xt,
        int input, FILE *output,
        TwinCellPtr screen, TwindowPtr root, TwinDamagePtr damage);


    void close_xterminator(XterminatorPtr xt);

    void resize_xterminator(XterminatorPtr xt);
    XterminatorPtr sync_xterminator(XterminatorPtr xt);
#ifdef __cplusplus
}
#endif                                 /* C++ */
#endif                                 /* XTERMINATOR_H */
