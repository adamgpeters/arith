/******************************************************************************
 *
 *                             compressinfo.h
 *
 *     Assignment: arith
 *     Authors:    Ryan Beckwith and Adam Peters
 *     Date:       10/27/2020
 *
 *     Defines macros containing information about the the width of chroma 
 *     and DCT values to be compressed and bitpacked. Further, externs an 
 *     enum containing information about the least significant bit of each 
 *     aforementioned value. 
 *
 *****************************************************************************/

#ifndef COMPRESSINFO_H
#define COMPRESSINFO_H

#define A_WIDTH 6
#define B_WIDTH 6
#define C_WIDTH 6
#define D_WIDTH 6
#define PB_WIDTH 4
#define PR_WIDTH 4

enum LSBS {
    a_lsb = 26, b_lsb = 20, c_lsb = 14, d_lsb = 8, pb_lsb = 4, pr_lsb = 0
}; 
extern enum LSBS lsbs;

#endif
