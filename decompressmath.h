/******************************************************************************
 *
 *                           decompressmath.h
 *
 *     Assignment: arith
 *     Authors:    Ryan Beckwith and Adam Peters
 *     Date:       10/27/2020
 *
 *     Interface for various decompression algorthims associated with the 
 *     decompression of a PPM file. (See decompressmath.c for more 
 *     information.)
 *
 *****************************************************************************/


#include "pnm.h"

#ifndef DECOMPRESSMATH_H
#define DECOMPRESSMATH_H

extern void cv_to_rgb(float chromas[3], float normalized_rgbs[3]);
extern float dequantize_avg_brightness(unsigned a);
extern void dct_to_brightness(float dcts[4], float y_vals[4]);
extern struct Pnm_rgb unscale_rgb(float normalized_rgbs[3],
                                  unsigned denominator);
extern float dequantize_dct(int quantized_dct);

#endif