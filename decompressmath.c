/******************************************************************************
 *
 *                             decompressmath.c
 *
 *     Assignment: arith
 *     Authors:    Ryan Beckwith and Adam Peters
 *     Date:       10/27/2020
 *
 *     Implements the decompressmath.h interface, which specifies various
 *     mathematical operations to convert component-video values to RGB values,
 *     as well as the inverse of a discrete cosine transform (DCT) from the
 *     brightness values of the component-video space. Designed for the purpose
 *     of implementing image decompression algorithms with these functions.
 *
 *****************************************************************************/

#include <stdlib.h>
#include "decompressmath.h"
#include "assert.h"

/*
 *  Function:  cv_to_rgb
 *  Arguments: float chromas[3] - an ordered array [y, pb, pr] that contains 
 *                                the chroma values for a pixel
 *             float normalized_rgbs[3] - an array of the form [r, g, b] to be  
 *                                        filled in with RGB values for a pixel
 *  Does:      Transforms chroma values for a pixel to RGB values. 
 *  Return:    void
 */
void cv_to_rgb(float chromas[3], float normalized_rgbs[3])
{
    assert(chromas != NULL && normalized_rgbs != NULL);

    /* converts from chroma values to normalized RGB values */
    normalized_rgbs[0] = 1.0 * chromas[0] + 0.0 * chromas[1] + 1.402 *
                         chromas[2];
    normalized_rgbs[1] = 1.0 * chromas[0] - 0.344136 * chromas[1] - 0.714136 *
                         chromas[2];
    normalized_rgbs[2] = 1.0 * chromas[0] + 1.772 * chromas[1] + 0.0 *
                         chromas[2];
}

/*
 *  Function:  dequantize_avg_brightness
 *  Arguments: float a - a float in the set {0, 1,... , 511} to be transformed
 *                       to the range [0, 1]
 *  Does:      Dequantizes a float in the set  {0, 1,... , 511} to the range
 *             [0, 1]
 *  Return:    float - the dequantized float
 */
float dequantize_avg_brightness(unsigned a)
{
    assert(a <= 511);
    return a / 63.0;
}

/*
 *  Function:  dequantize_dct
 *  Arguments: int dct_val - a quantized integer DCT value in the set 
 *             {−15, −14,... , 15} 
 *  Does:      Dequantizes an integer DCT value to the range [-0.3, +0.3]  
 *  Return:    float - the dequantized DCT value in the range [-0.3, +0.3]. 
 */
float dequantize_dct(int quantized_dct)
{
    assert(quantized_dct >= -15 && quantized_dct <= 15);
    return quantized_dct / 50.0;
}

/*
 *  Function:  dct_to_brightness
 *  Arguments: float dcts[4] - an ordered array [a, b, c, d] containing the
 *                             corresponding DCT-space values
 * 
 * 
 *             float y_vals[4] - an ordered array [y1, y2, y3, y4] to be filled
 *                               with Y (brightness) values for each of
 *                               the four pixels in a 2x2 block
 *             
 *  Does:      Converts from DCT space to pixel space (Y values), storing the
 *             resulting pixel space values in the y_vals ordered array.
 *  Return:    void
 */
void dct_to_brightness(float dcts[4], float y_vals[4])
{
    assert(dcts != NULL && y_vals != NULL);
    float a = dcts[0];
    float b = dcts[1];
    float c = dcts[2];
    float d = dcts[3];

    /* computes corresponding brightness values from DCT space */
    y_vals[0] = a - b - c + d;
    y_vals[1] = a - b + c - d;
    y_vals[2] = a + b - c - d;
    y_vals[3] = a + b + c + d;
}

/*
 *  Function:  unscale_rgb
 *  Arguments: float normalized_rgbs[3] - an ordered array [r, g, b] that  
 *                                        contains RGB values for a pixel in 
 *                                        the range [0, 1]
 *             unsigned denominator - the desired denominator used to upscale
 *                                    each of the pixels
 *  Does:      Upscales the specified RGB values by the desired denominator,
 *             storing each in a Pnm_rgb struct which is returned as a struct
 *             literal.
 *  Return:    Pnm_rgb - a struct literal containing the upscaled, unsigned RGB
 *                       values for the specified pixel
 */
struct Pnm_rgb unscale_rgb(float normalized_rgbs[3], unsigned denominator)
{
    assert(normalized_rgbs != NULL);

    /* upscales the RGB values by the specified denominator */
    struct Pnm_rgb pixel = { .red = normalized_rgbs[0] * denominator,
                             .green = normalized_rgbs[1] * denominator,
                             .blue = normalized_rgbs[2] * denominator
                           };
    return pixel;
}
