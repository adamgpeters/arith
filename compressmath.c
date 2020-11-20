/******************************************************************************
 *
 *                              compressmath.c
 *
 *     Assignment: arith
 *     Authors:    Ryan Beckwith and Adam Peters
 *     Date:       10/27/2020
 *
 *     Implements the compressmath.h interface, which specifies various
 *     mathematical operations to convert RGB values to component-video space,
 *     as well as a discrete cosine transform (DCT) from the brightness values
 *     of the component-video space. Designed for the purpose of implementing
 *     image compression algorithms with these functions.
 *
 *****************************************************************************/

#include <stdlib.h>

#include "assert.h"

#include "math.h"
#include "pnm.h"
#include "compressmath.h"

/*
 *  Function:  rgb_to_cv
 *  Arguments: float normalized_rgbs[3] - an ordered array [r, g, b] that  
 *                                        contains RGB values for a pixel in 
 *                                        the range [0, 1]
 *             float chromas[3] - an array of the form [y, pb, pr] to be 
 *                                filled in with chroma values for a pixel
 *  Does:      Transforms RGB values for a pixel to chroma values. 
 *  Return:    void
 */
void rgb_to_cv(float normalized_rgbs[3], float chromas[3])
{
    assert(normalized_rgbs != NULL && chromas != NULL);
    float red = normalized_rgbs[0];
    float green = normalized_rgbs[1];
    float blue = normalized_rgbs[2];

    /* calculate chroma values */
    float y = 0.299 * red + 0.587 * green + 0.114 * blue;
    float pb = -0.168736 * red - 0.331264 * green + 0.5 * blue;
    float pr = 0.5 * red - 0.418688 * green - 0.081312 * blue;

    /* store chroma values in corresponding positions of the chromas array */
    chromas[0] = y;
    chromas[1] = pb;
    chromas[2] = pr;
}

/*
 *  Function:  quantize_avg_brightness
 *  Arguments: float a - a float in the range [0, 1] to be quantized. 
 *  Does:      Quantizes a float in the range [0, 1] to the discrete set 
 *             {0, 1,... , 511}
 *  Return:    unsigned - the quantized float
 */
unsigned quantize_avg_brightness(float a) 
{  
    assert(a >= 0.0 && a <= 1.0);
    return round(a * 63);
}

/*
 *  Function:  quantize_dct
 *  Arguments: float dct_val - a DCT value to be quantized
 *  Does:      Quantizes a float DCT value to the discrete set 
 *             {−15, −14,... , 15}. 
 *  Return:    int - the quantized DCT value in the set {−15, −14,... , 15}  
 *                   where any DCT value less than -0.3 is mapped to -15,  
 *                   and anything greater than 0.3 is mapped to 15. 
 */
int quantize_dct(float dct_val) 
{
    int quantized;

    /* ensure that values of larger magnitude than 0.3 are quantized to 15 or
       -15*/
    if (dct_val <= -0.3) {
        quantized = -15;
    } else if (dct_val >= 0.3) {
        quantized = 15;
    } else {
        /* quantize values between -0.3 and 0.3 (exclusive) */
        quantized = round(dct_val * 50);
    }
    return quantized;
}

/*
 *  Function:  pix_to_dct
 *  Arguments: float y_vals[4] - an ordered array [y1, y2, y3, y4] which
 *                               contains the Y (brightness) values for each of
 *                               the four pixels in a 2x2 block
 *             float dcts[4] - an ordered array [a, b, c, d] to be filled with
 *                             the corresponding DCT-space values
 *  Does:      Converts from pixel space (Y values) to DCT space, storing the
 *             resulting DCT values in the dcts ordered array.
 *  Return:    void
 */
void pix_to_dct(float y_vals[4] , float dcts[4])
{
    assert(y_vals != NULL && dcts != NULL);

    /* calculate each DCT value and store in the correct position of dcts */
    dcts[0] = (y_vals[3] + y_vals[2] + y_vals[1] + y_vals[0]) / 4.0;
    dcts[1] = (y_vals[3] + y_vals[2] - y_vals[1] - y_vals[0]) / 4.0;
    dcts[2] = (y_vals[3] - y_vals[2] + y_vals[1] - y_vals[0]) / 4.0; 
    dcts[3] = (y_vals[3] - y_vals[2] - y_vals[1] + y_vals[0]) / 4.0;
}

/*
 *  Function:  scale_rgb
 *  Arguments: Pnm_rgb pixel - a pointer to an existing struct containing RGB
 *                             information about a pixel.
 *             unsigned denominator - the denominator of the PPM image from
 *                                    which values of pixel are derived
 *             float normalized_rgbs[3] - an ordered array [r, g, b] that  
 *                                        contains RGB values for a pixel in 
 *                                        the range [0, 1]
 *  Does:      Scales each specified RGB value into the range [0, 1] and stores
 *             each value in the corresponding position of normalized_rgbs.
 *  Return:    void
 */
void scale_rgb(Pnm_rgb pixel, unsigned denominator, float normalized_rgbs[3])
{
    assert(pixel != NULL && normalized_rgbs != NULL);
    float denom = (float)denominator;

    /* scale down by the denominator */
    normalized_rgbs[0] = pixel->red / denom;
    normalized_rgbs[1] = pixel->green / denom;
    normalized_rgbs[2] = pixel->blue / denom;
}
