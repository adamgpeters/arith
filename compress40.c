/******************************************************************************
 *
 *                               compress40.c
 *
 *     Assignment: arith
 *     Authors:    Ryan Beckwith and Adam Peters
 *     Date:       10/27/2020
 *
 *     Implements the compress40 function (whose contract is provided in
 *     compress40.h), which compresses a provided PPM image and writes the
 *     compressed PPM image to stdout. Each 2x2 pixel group in the source PPM
 *     image maps to a single 32-bit word in the compressed image. Images with
 *     odd dimensions are truncated down to even dimensions.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "assert.h"

#include "compress40.h"
#include "a2methods.h"
#include "pnm.h"
#include "arith40.h"

#include "compressmath.h"
#include "a2blocked.h"
#include "a2plain.h"
#include "uarray2b.h"
#include "uarray2.h"
#include "bitpack.h"
#include "compressinfo.h"

/* Mapping closure struct declaration, implementation, and pointer typedef */
typedef struct Compression_Info {
    UArray2_T compressed;
    float *y_vals;
    float avg_pr;
    float avg_pb;
    unsigned denominator;
    unsigned orig_width;
    unsigned orig_height;
} *Compression_Info;

/* Static function declarations */
static uint32_t bitpack_pixels(unsigned a, int b, int c, int d,
                               unsigned avg_pb_ind, unsigned avg_pr_ind);
static void compress_cb(int col, int row, A2Methods_UArray2 image, void *elem,
                        void *cl);
static void write_compressed(UArray2_T compressed);
static void print_compress_cb(int col, int row, UArray2_T compressed,
                              void *elem, void *cl);
static void print_big_endian(uint32_t word);
static void pack_pixel(Compression_Info c_info, int col, int row);

/*
 *  Function:  write_compressed
 *  Arguments: UArray2_T compressed - a pointer to an existing 2d array
 *                                    containing bitpacked pixel groups
 *  Does:      Writes the bytes of a compressed PPM file to stdout in Big 
 *             Endian order. 
 *  Return:    void
 */
static void write_compressed(UArray2_T compressed)
{
    assert(compressed != NULL);
    printf("COMP40 Compressed image format 2\n%u %u\n",
           UArray2_width(compressed), UArray2_height(compressed));
    
    /* print all bytes in compressed in big endian order */
    UArray2_map_row_major(compressed, print_compress_cb, NULL);
}

/*
 *  Function:  print_compress_cb
 *  Arguments: int col - the column index of a bitpacked pixel group in the
 *                       specified 2D array (unnessary for function, voided)
 *             int row - the column index of a bitpacked pixel group in the
 *                       specified 2D array (unnessary for function, voided)
 *             UArray2_T compressed - a pointer to an existing 2d array 
 *                                    containing bitpacked pixel groups
 *                                    (unnessary for function, voided)
 *             void *elem - pointer to an element in the compressed 2d array
 *             void *cl - a pointer to some closure. (unnessary for function,
 *                        voided)
 *  Does:      Callback function used to write the bytes of a compressed PPM 
 *             file to stdout in Big Endian order. 
 *  Return:    void
 */
static void print_compress_cb(int col, int row, UArray2_T compressed,
                              void *elem, void *cl)
{
    assert(elem != NULL);
    (void)col;
    (void)row;
    (void)compressed;
    (void)cl;
    print_big_endian(*(uint32_t *)elem);
}

/*
 *  Function:  print_big_endian
 *  Arguments: uint32_t word - a bitpacked, 32-bit word
 *  Does:      Outputs (with putchar) each byte in the word in big endian 
 *             order. 
 *  Return:    void
 */
static void print_big_endian(uint32_t word)
{   
    for (int i = (int)(sizeof word) - 1; i >= 0; i--)
    {
        putchar(Bitpack_getu(word, 8, i * 8));
    }
}

/*
 *  Function:  bitpack_pixels
 *  Arguments: unsigned a - quantized DCT a value
 *             int b - quantized DCT b value
 *             int c - quantized DCT c value
 *             int d - quantized DCT d value
 *             unsigned pb_ind - index of chroma pb value in external table
 *             unsigned pr_ind - index of chroma pr value in external table
 *  Does:      Bitpacks DCT and chroma values into a uint32_t. Each value 
 *             has a width and lsb specified in compressinfo.h.
 *  Return:    a uint32_t with all arguments bitpacked in it
 */
static uint32_t bitpack_pixels(unsigned a, int b, int c, int d,
                               unsigned pb_ind, unsigned pr_ind)
{
    uint64_t word = 0;
    word = Bitpack_newu(word, A_WIDTH, a_lsb, a);
    word = Bitpack_news(word, B_WIDTH, b_lsb, b);
    word = Bitpack_news(word, C_WIDTH, c_lsb, c);
    word = Bitpack_news(word, D_WIDTH, d_lsb, d);
    word = Bitpack_newu(word, PB_WIDTH, pb_lsb, pb_ind);
    word = Bitpack_newu(word, PR_WIDTH, pr_lsb, pr_ind);
    return word;
}

/*
 *  Function:  pack_pixel
 *  Arguments: Compression_Info c_info - Stores information necessary to store
 *                                       bitpacked pixel groups in a 2D array
 *                                       representing the compressed image.
 *             int col - the column index of a pixel in the specified 2D array
 *             int row - the row index of a pixel in the specified 2D array
 *  Does:      Bitpacks all necessary quantized information about four pixels
 *             into a single 32-bit word, which is subsequently stored in a 2D
 *             array representing the compressed image. This 2D array is stored
 *             in the c_info variable.
 *  Return:    void
 */
static void pack_pixel(Compression_Info c_info, int col, int row)
{
    assert(c_info != NULL);
    /* get DCT values */
    float dcts[4];
    pix_to_dct(c_info->y_vals, dcts);
    
    /* quantize DCT values */
    unsigned a = quantize_avg_brightness(dcts[0]);
    unsigned b = quantize_dct(dcts[1]);
    unsigned c = quantize_dct(dcts[2]);
    unsigned d = quantize_dct(dcts[3]);

    /* get average chroma values */
    float avg_pb = c_info->avg_pb / 4.0;
    float avg_pr = c_info->avg_pr / 4.0;

    /* bitpack dcts and index of chromas */
    uint32_t bitpacked_data = bitpack_pixels(a, b, c, d,
                                             Arith40_index_of_chroma(avg_pb),
                                             Arith40_index_of_chroma(avg_pr));
    
    /* place bitpacked data into compressed 2d array */
    *(uint32_t *)UArray2_at(c_info->compressed, col / 2, row / 2) = 
        bitpacked_data;

    /* reset closure for reading new pixels */
    c_info->avg_pb = 0;
    c_info->avg_pr = 0;
}

/*
 *  Function:  compress40
 *  Arguments: int col - the column index of a pixel in the specified 2D array
 *             int row - the row index of a pixel in the specified 2D array
 *             A2Methods_UArray2 image - a pointer to an existing 2D array
 *                                       containing all RGB information from a
 *                                       PPM image file
 *             void *elem - void pointer to an existing Ppm_rgb instance, which
 *                          represents the RGB data for a single pixel
 *             void *cl - void pointer to a closure variable containing all
 *                        necessary additional info to compress each pixel in
 *                        image, the provided 2D array
 *  Does:      Callback function which maps a grouping of four pixels to a
 *             bitpacked 32-bit representation. Each word is stored in an
 *             externally defined 2D array specified in the closure instance.
 *             (represented by RGB values)
 *  Return:    void
 */
static void compress_cb(int col, int row, A2Methods_UArray2 image, void *elem,
                        void *cl)
{
    assert(image != NULL && elem != NULL && cl != NULL);
    Compression_Info c_info = (Compression_Info)cl;
    assert(c_info != NULL && c_info->y_vals != NULL);

    /* ensure that only pixel groups with exactly four pixels are read */
    if ((unsigned)col >= c_info->orig_width - c_info->orig_width % 2 ||
        (unsigned)row >= c_info->orig_height - c_info->orig_height % 2) {
        return;
    }
    
    /* scale rgb to the range [0, 1] */
    Pnm_rgb rgb = (Pnm_rgb)elem;
    float normalized_rgbs[3];
    scale_rgb(rgb, c_info->denominator, normalized_rgbs);

    /* get chroma values */
    float chromas[3];
    rgb_to_cv(normalized_rgbs, chromas);
    c_info->avg_pb += chromas[1];
    c_info->avg_pr += chromas[2];

    /* get pixel space value, y, and write into the pixel group's array of 
       y values */
    int y_val_pos = (row % 2) * 2 + (col % 2);
    c_info->y_vals[y_val_pos] = chromas[0];

    /* only pack pixel group info into compressed image on the fourth one */
    if (row % 2 == 1 && col % 2 == 1) {
        pack_pixel(c_info, col, row);
    }
}

/*
 *  Function:  compress40
 *  Arguments: FILE *input - a non-null pointer to an opened PPM image file
 *  Does:      Compresses a provided PPM file and writes the compressed PPM to
 *             stdout. Does not close the provided FILE pointer. 
 *  Return:    void
 */
void compress40(FILE *input)
{
    assert(input != NULL);
    
    /* use methods for a blocked 2D array */
    A2Methods_T methods = uarray2_methods_blocked;

    /* store the pixels in a blocked 2D array with a blocksize of 2 (new method
       of uarray2_methods_blocked defaults to 2 instead of the maximum size) */
    Pnm_ppm image = Pnm_ppmread(input, methods);
    assert(image != NULL && image->pixels != NULL);

    /* create the compressed image unboxed 2D array */
    UArray2_T compressed = UArray2_new(image->width / 2, image->height / 2,
                                       sizeof(uint32_t));

    /* map across each 2x2 block and compress/store each block */
    float y_vals[4];
    struct Compression_Info c_info = {compressed, y_vals, 0, 0,
                                      image->denominator, image->width,
                                      image->height};
    methods->map_block_major(image->pixels, compress_cb, &c_info);

    /* write the compressed image to stdout and free heap-allocated memory */
    write_compressed(compressed);
    UArray2_free(&compressed);
    Pnm_ppmfree(&image);
}
