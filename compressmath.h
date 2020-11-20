/******************************************************************************
 *
 *                              compressmath.h
 *
 *     Assignment: arith
 *     Authors:    Ryan Beckwith and Adam Peters
 *     Date:       10/27/2020
 *
 *     Interface for various compression algorithms associated with the
 *     compression of a PPM image. (See the implementation file compressmath.c
 *     for more information)
 *
 *****************************************************************************/

#include "pnm.h"

#ifndef COMPRESSMATH_H
#define COMPRESSMATH_H

extern void rgb_to_cv(float normalized_rgbs[3], float chromas[3]);
extern unsigned quantize_avg_brightness(float a);
extern int quantize_dct(float dct_val);
extern void pix_to_dct(float y_vals[4] , float dcts[4]);
extern void scale_rgb(Pnm_rgb pixel, unsigned denominator, 
                      float normalized_rgbs[3]);

#endif