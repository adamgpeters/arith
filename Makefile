# Makefile for arith (Comp 40 Assignment 4)
# 
# Authors: Ryan Beckwith and Adam Peters
# Date:    10/27/2020
#
# Includes build rules for bitpack.o and 40image. Based off of previously
# provided Makefiles from past Comp 40 assignments.


############## Variables ###############

CC = gcc # The compiler being used

# Updating include path to use Comp 40 .h files and CII interfaces
IFLAGS = -I. -I/comp/40/build/include -I/usr/sup/cii40/include/cii

# Compile flags
# Set debugging information, allow the c99 standard,
# max out warnings, and use the updated include path
CFLAGS = -g -std=c99 -Wall -Wextra -Werror -Wfatal-errors -pedantic $(IFLAGS)

# Linking flags
# Set debugging information and update linking path
# to include course binaries and CII implementations
LDFLAGS = -g -L/comp/40/build/lib -L/usr/sup/cii40/lib64

# Libraries needed for linking
LDLIBS = -lcii40 -lm -lnetpbm -lpnm -larith40

# Collect all .h files in your directory.
INCLUDES = $(shell echo *.h)

# Standard compilation script extracted into a variable for convenience
COMPILE = $(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

############### Rules ###############

# Makes all files required for this assignment
all: bitpack.o 40image-6

# To get *any* .o file, compile its .c file with the following rule.
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@

40image-6: 40image.o compress40.o decompress40.o a2blocked.o a2plain.o \
		 uarray2b.o uarray2.o compressmath.o decompressmath.o bitpack.o
	$(COMPILE)

# Removes .o files, as well as executables, from current working directory
clean:
	rm -f 40image *.o