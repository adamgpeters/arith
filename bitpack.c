/******************************************************************************
 *
 *                                bitpack.c
 *
 *     Assignment: arith
 *     Authors:    Ryan Beckwith and Adam Peters
 *     Date:       10/27/2020
 *
 *     Implements the bitpack.h interface, which provides the ability to pack
 *     and unpack signed and unsigned field values into 64-bit unsigned
 *     integers. Also implements functions to check if a specified
 *     unsigned/signed value can be represented with a specified number of
 *     bits.
 *
 *****************************************************************************/

#include "bitpack.h"
#include <stdlib.h>
#include <stdio.h>

#include "assert.h"
#include "except.h"

/* Static function declarations */
static uint64_t rshift(uint64_t val, unsigned shift_amt);
static uint64_t lshift(uint64_t val, unsigned shift_amt);
static uint64_t pow2(unsigned exp);

/* Exception type for overflow errors */
Except_T Bitpack_Overflow = { "Overflow packing bits" };

/*
 *  Function:  rshift
 *  Arguments: uint64_t val - value to be right shifted
 *             unsigned shift_amt - shift amount
 *  Does:      Right shifts a uint64_t by a specific value and standarizes
 *             shifts bigger than the value's size (64).
 *  Return:    the uint64_t created by val >> shift_amt if shift_amt is less
 *             than 64, return 0 otherwise
 */
static uint64_t rshift(uint64_t val, unsigned shift_amt)
{
    if (shift_amt >= 64) {
        return 0;
    }
    return val >> shift_amt;
}

/*
 *  Function:  lshift
 *  Arguments: uint64_t val - value to be left shifted
 *             unsigned shift_amt - shift amount
 *  Does:      Left shifts a uint64_t by a specific value and standarizes
 *             shifts bigger than the value's size (64).
 *  Return:    uint64_t - the uint64_t created by val << shift_amt if shift_amt
 *             is less than 64, returns 0 otherwise
 */
static uint64_t lshift(uint64_t val, unsigned shift_amt)
{
    if (shift_amt >= 64) {
        return 0;
    }
    return val << shift_amt;
}

/*
 *  Function:  pow2
 *  Arguments: unsigned exp - exponent by which 2 will be raised
 *  Does:      Finds two raised to some number, but gets 0 if the exponent
 *             is greater than or equal to 64. 
 *  Return:    uint64_t the uint64_t created by 2 ^ exp if exp is less than 64,
 *             returns 0 othersize
 */
static uint64_t pow2(unsigned exp)
{
    return lshift(1, exp);
}

/*
 *  Function:  Bitpack_fitsu
 *  Arguments: uint64_t n - the unsigned value that is tested to determine if
 *                          it can fit in width bits
 *             unsigned width - number of bits in which n is tested to fit
 *  Does:      Determines if the unsigned number n can be represented in 
 *             width bits. Note: when width is greater than or equal to 64, 
 *             any uint64_t n can fit within those bits. 
 *  Return:    bool - 1 if n can be represented in width bits and 0 otherwise
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{   
    /* edge case if width >= 64 */
    if (width >= 64) {
        return true;
    }
    
    uint64_t num_vals = pow2(width);
    if (num_vals > n) {
        return true;
    } 
    return false;
}

/*
 *  Function:  Bitpack_fitss
 *  Arguments: uint64_t n - the signed value that is tested to determine if
 *                          it can fit in width bits
 *             unsigned width - number of bits in which n is tested to fit
 *  Does:      Determines if the signed number n can be represented in 
 *             width bits. Note: when width is greater than or equal to 64, 
 *             any uint64_t n can fit within those bits. 
 *  Return:    bool - 1 if n can be represented in width bits and 0 otherwise
 */
bool Bitpack_fitss(int64_t n, unsigned width)
{
    /* edge case if width is 0 or width is >= 64 */
    if (width == 0) {
        return false;
    } else if (width >= 64) {
        return true;
    }

    /* get the range of possible values that can be represented by width bits
       for a signed two's complement integer */
    uint64_t num_vals = pow2(width - 1);
    if (-1 * (int64_t)num_vals <= n && (int64_t)num_vals > n) {
        /* return true if n is within this range */
        return true;
    }
    /* return false otherwise */
    return false;
}

/*
 *  Function:  Bitpack_getu
 *  Arguments: uint64_t word - the word in which data will be extracted
 *             unsigned width - the number of bits to be extracted from the
 *                              word
 *             unsigned lsb - the least significant bit of the field to be 
 *                            extracted
 *  Does:      Extracts a specified, unsigned field from a uint64_t word. 
 *  Return:    uint64_t - a field extracted from word of size width with a
 *                        least significant bit of lsb 
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
    assert(width <= 64 && width + lsb <= 64);

    /* create a mask by right-shifting 1s by 64 - width */
    uint64_t mask = rshift(~(uint64_t)0, sizeof(uint64_t) * 8 - width);

    /* left-shift mask to start of desired field */
    mask = lshift(mask, lsb);

    /* bitwise AND mask with word to obtain field value, then right-shift back
       to start of integer to get actual value */
    return rshift(mask & word, lsb);
}

/*
 *  Function:  Bitpack_gets
 *  Arguments: uint64_t word - the word in which data will be extracted
 *             unsigned width - the number of bits to be extracted from the 
 *                              word
 *             unsigned lsb - the least significant bit of the field to be 
 *                            extracted
 *  Does:      Extracts a specified, signed field from a uint64_t word. 
 *  Return:    int64_t - a field extracted from word of size width with a
 *                        least significant bit of lsb 
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
    assert(width <= 64 && width + lsb <= 64);
    
    /* edge case if width is 0 */
    if (width == 0) {
        return 0;
    }

    int64_t result;
    /* check to see if the field being extracted is positive or negative */
    if (Bitpack_getu(word, 1, lsb + width - 1) == 1) {
        /* edge case if negative and width is 64 */
        if (width == 64) {
            return (int64_t)word;
        }
        /* calculate the value of a two's complement signed integer to obtain a
           negative result */
        result = (-1 * (int64_t)pow2(width - 1)) +
                 Bitpack_getu(word, width - 1, lsb);
    } else {
        /* use the previously created function to extract a positive value */
        result = Bitpack_getu(word, width, lsb);
    }
    return result;
}

/*
 *  Function:  Bitpack_newu
 *  Arguments: uint64_t word - the existing 64-bit word to be updated
 *             unsigned width - the number of bits in the field to be updated
 *                              in word, which must be less than 64
 *             unsigned lsb - the index of the least significant bit of the
 *                            field to be updated in word
 *             int64_t value - the unsigned integer value to store in the
 *                             specified field in word
 *  Does:      Updates the value of a specified field in the provided word,
 *             clearing the previous value and replacing it with a new one.
 *             Requires that width + lsb is less than or equal to 64.
 *  Return:    uint64_t - the bitpacked word with an updated field in unsigned
 *                        representation
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value)
{
    assert(width <= 64 && width + lsb <= 64);

    /* error case if the value does not fit in the specified width */
    if (!Bitpack_fitsu(value, width)) {
        RAISE(Bitpack_Overflow);
    }

    /* create a mask by right-shifting 1s by 64 - width */
    uint64_t mask = rshift(~(uint64_t)0, sizeof(uint64_t) * 8 - width);
    /* shift the mask left to the start of the field */
    mask = lshift(mask, lsb);

    /* update the word to contain all 0s in the desired field */
    uint64_t updated_word = word & (~mask);
    
    /* update the desired field by bitwise ORing with the left-shifted value */
    return lshift(value, lsb) | updated_word;
}

/*
 *  Function:  Bitpack_news
 *  Arguments: uint64_t word - the existing 64-bit word to be updated
 *             unsigned width - the number of bits in the field to be updated
 *                              in word, which must be less than 64
 *             unsigned lsb - the index of the least significant bit of the
 *                            field to be updated in word
 *             int64_t value - the integer value to store in the specified
 *                             field in word
 *  Does:      Updates the value of a specified field in the provided word,
 *             clearing the previous value and replacing it with a new one.
 *             Requires that width + lsb is less than or equal to 64.
 *  Return:    uint64_t - the bitpacked word with an updated field in unsigned
 *                        representation
 */
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
                      int64_t value)
{
    assert(width <= 64 && width + lsb <= 64);
    /* error case if the value does not fit in the specified width */
    if (!Bitpack_fitss(value, width)) {
        RAISE(Bitpack_Overflow);
    }

    /* create a mask by right-shifting 1s by 64 - width */
    uint64_t mask = rshift(~(uint64_t)0, sizeof(uint64_t) * 8 - width);
    /* shift the mask left to the start of the field */
    mask = lshift(mask, lsb);

    /* update the word to contain all 0s in the desired field */
    uint64_t updated_word = word & (~mask);

    /* update the word by adding value to the mask, then bitwise ORing with
       updated_word */
    return (mask & lshift(value, lsb)) | updated_word;
}
