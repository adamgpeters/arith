/******************************************************************************
 *
 *                               decompress40.c
 *
 *     Assignment: arith
 *     Authors:    Ryan Beckwith and Adam Peters
 *     Date:       10/27/2020
 *
 *     Implements the decompress40 function (whose contract is provided in
 *     compress40.h), which decompresses a provided compressed PPM image and
 *     writes the decompressed PPM image to stdout. Each 32-bit word in the
 *     compressed PPM image maps to a 2x2 pixel group in the decompressed
 *     image.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdlib.h>
#include <stdio.h>

#include "assert.h"

#include "pnm.h"
#include "a2methods.h"
#include "arith40.h"
#include "compress40.h"

#include "a2blocked.h"
#include "a2plain.h"
#include "uarray2.h"
#include "uarray2b.h"
#include "bitpack.h"
#include "decompressmath.h"
#include "compressinfo.h"

/* Static function declarations */
static void decompress_cb(int col, int row, UArray2_T image,
                          void *elem, void *cl);
static UArray2_T read_compressed(FILE *input);
static void trim_normalized_rgbs(float normalized_rgbs[3]);
static void decompress_pixel(float avg_pb, float avg_pr, float y_vals[4],
                             Pnm_ppm pixmap, int col, int row);
    
/*
 *  Function:  decompress40
 *  Arguments: FILE *input - a non-null pointer to an opened, compressed PPM 
 *                          image file
 *  Does:      Decompresses a compressed PPM file and writes that new PPM
 *             to stdout. Does not close the provided FILE pointer. 
 *  Return:    void
 */
void decompress40(FILE *input)
{
    assert(input != NULL);
    UArray2_T compressed = read_compressed(input);

    A2Methods_T methods = uarray2_methods_blocked;
    Pnm_rgb temp;
    A2Methods_UArray2 pixels = methods->new(UArray2_width(compressed) * 2,
                                            UArray2_height(compressed) * 2,
                                            sizeof(*temp));
    unsigned denominator = 255; 
    /* create PPM for the decompressed image */
    struct Pnm_ppm pixmap = { .width = methods->width(pixels),
                              .height = methods->height(pixels),
                              .denominator = denominator, .pixels = pixels,
                              .methods = methods
                            };
    Pnm_ppm pixmap_p = &pixmap;
    
    /* map over compressed PPM and decompress it into pixmap_p */
    UArray2_map_row_major(compressed, decompress_cb, pixmap_p);
    
    /* write decompressed PPM to stdout */
    Pnm_ppmwrite(stdout, pixmap_p);

    /* free heap allocated memory (except *input) */
    methods->free(&pixels);
    UArray2_free(&compressed);
}

/*
 * Function:  trim_normalized_rgbs
 * Arguments: float normalized_rgbs[3] - a float array of three rgb values
 * Does:      Trims each rgb value to the range [0, 1]. All values less than 0,
 *            are normalized to 0, and all values greater than 1 are normalized
 *            to 1. 
 * Return:    void
 */
static void trim_normalized_rgbs(float normalized_rgbs[3])
{   
    for (int i = 0; i < 3; i++) {
        normalized_rgbs[i] = normalized_rgbs[i] < 0 ? 0: normalized_rgbs[i];
        normalized_rgbs[i] = normalized_rgbs[i] > 1 ? 1: normalized_rgbs[i];
    }
}

/*
 * Function:  decompress_pixel
 * Arguments: float avg_pb - average Pb chroma value for a bitpacked pixel 
 *                           group
 *            float avg_pr - average Pr chroma value for a bitpacked pixel 
 *                           group
 *            float y_vals[4] - brightness values for a bitpacked pixel group
 *            Pnm_ppm pixmap - a pointer to the decompressed ppm 
 *            int col - the column index of a bitpacked pixel group in the 
 *                      specified 2D array
 *            int row - the row index of a bitpacked pixel group in the 
 *                      specified 2D array
 * Does:      Transforms chroma values for a bitpacked pixel group located at
 *            (col, row) into their four corresponding rgb values, and then
 *            places those values into their position in pixmap.
 * Return:    void
 */
static void decompress_pixel(float avg_pb, float avg_pr, float y_vals[4],
                             Pnm_ppm pixmap, int col, int row)
{
    assert(y_vals != NULL && pixmap != NULL);
    float chromas[3] = {0, avg_pb, avg_pr};
    float normalized_rgbs[3];
    /* place pixel group into decompressed PPM */
    for (int i = 0; i < 4; i++) {
        chromas[0] = y_vals[i];
        cv_to_rgb(chromas, normalized_rgbs);
        trim_normalized_rgbs(normalized_rgbs);
        struct Pnm_rgb pixel = unscale_rgb(normalized_rgbs, 
                                           pixmap->denominator);

        /* place pixel at is corresponding row and col in the decompressed 
           image */
        Pnm_rgb dest_elem = pixmap->methods->at(pixmap->pixels, 
                                                (col * 2) + i % 2, 
                                                (row * 2) + i / 2);
        *dest_elem = pixel;
    }
}

/*
 * Function:  decompress_cb
 * Arguments: int col - the column index of a bitpacked pixel group in the
 *                      specified 2D array
 *            int row - the row index of a bitpacked pixel group in the
 *                      specified 2D array
 *            UArray2_T image - a pointer to an existing 2D array containing
 *                              each bitpacked pixel group from the
 *                              compressed image.
 *            void *elem - a void pointer to the current bitpacked pixel group
 *            void *cl - a void pointer to a Pnm_ppm struct representing the
 *                       decompressed destination image
 * Does:      Callback function that maps a bitpacked pixel group to a set of
 *            four pixels in a destination image array.
 * Return:    void
 */
static void decompress_cb(int col, int row, UArray2_T image, void *elem,
                          void *cl) 
{
    assert(image != NULL && elem != NULL && cl != NULL);
    uint32_t word = *(uint32_t *)elem;
    Pnm_ppm pixmap = (Pnm_ppm)cl;
    assert(pixmap->pixels != NULL && pixmap->methods != NULL);
    
    /* unbitpack and dequantize pixel data */
    float dq_a = dequantize_avg_brightness(Bitpack_getu(word, A_WIDTH, a_lsb));
    float dq_b = dequantize_dct(Bitpack_gets(word, B_WIDTH, b_lsb));
    float dq_c = dequantize_dct(Bitpack_gets(word, C_WIDTH, c_lsb));
    float dq_d = dequantize_dct(Bitpack_gets(word, D_WIDTH, d_lsb));
    unsigned pb_index = Bitpack_getu(word, PB_WIDTH, pb_lsb);
    unsigned pr_index = Bitpack_getu(word, PR_WIDTH, pr_lsb);
    float avg_pb = Arith40_chroma_of_index(pb_index);
    float avg_pr = Arith40_chroma_of_index(pr_index);
 
    /* transform DCT space to brighness values */
    float dcts[4] = {dq_a, dq_b, dq_c, dq_d};
    float y_vals[4];
    dct_to_brightness(dcts, y_vals);

    /* tranform chroma values into RGB space and place in pixmap */
    decompress_pixel(avg_pb, avg_pr, y_vals, pixmap, col, row);
}

/*
 *  Function:  read_header
 *  Arguments: FILE *input - a non-null pointer to an opened, compressed PPM
 *                           image file
 *             unsigned *width - A pointer to an existing integer variable 
 *                               that will store the width of the image.
 *             unsigned *height - A pointer to an existing integer variable 
 *                                that will store the height of the image.
 *  Does:      Reads the header metadata from the specified compressed image
 *             file, which must include the width and height of the image. Sets
 *             the values of the dereferenced width and height pointers. Checks
 *             for invalid dimensions and other improper file types.
 *  Return:    void
 */
static void read_header(FILE *input, unsigned *width, unsigned *height)
{
    assert(input != NULL && width != NULL && height != NULL);
    /* check for a direct match of the specified phrase, store width/height */
    int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", width,
                      height);
    assert(read == 2);
    /* error case if width or height is nonpositive */
    if (*width == 0 || *height == 0) {
        fprintf(stderr, "Invalid compressed image dimensions.\n");
        fclose(input);
        exit(EXIT_FAILURE);
    }
    int c = getc(input);
    assert(c == '\n');
}

/*
 *  Function:  read_compressed
 *  Arguments: FILE *input - a non-null pointer to an opened, compressed PPM
 *                           image file
 *  Does:      Reads compressed data from the specified image file, storing each
 *             word in an unboxed 2D array. A pointer to this array is returned
 *             to the client, which must be manually freed by the client before
 *             program termination.
 *  Return:    UArray2_T - the array containing the compressed words.
 */
static UArray2_T read_compressed(FILE *input)
{
    /* read width and height from header */
    unsigned height, width;
    read_header(input, &width, &height);
    
    /* create 2D array to store bitpacked pixel groups */
    UArray2_T compressed = UArray2_new(width, height, sizeof(uint32_t));

    /* loop over expected size of image store each word in compressed */
    int c = getc(input);
    for (unsigned row = 0; row < height; row++) {
        for (unsigned col = 0; col < width; col++) {
            uint32_t word = 0;
            /* obtain each byte accounting for big endianness */
            for(int byte = 3; byte >= 0 && !feof(input); c = getc(input)) {
                word = Bitpack_newu(word, 8, byte * 8, c);
                byte--;
            }
            /* error case if there were not enough pixels provided */
            if (feof(input) && !(row == height - 1 && col == width - 1)) {
                fprintf(stderr, "Invalid compressed image file.\n");
                fclose(input);
                UArray2_free(&compressed);
                exit(EXIT_FAILURE);
            }
            /* store bitpacked word in compressed */
            *(uint32_t *)UArray2_at(compressed, col, row) = word;
        }
    }
    return compressed;
}
