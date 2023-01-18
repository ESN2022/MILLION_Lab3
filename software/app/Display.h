#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "ADXL345.h"

/* Display macros and constants used to program the SOPC */

// NOTE: in an ideal world, those macros should have been generated from the 
// QSYS editor and be embedded within the HAL.

// Display extended (i.e. may not only be numbers [0-9]) digits in the same 
// order they are entered.
// NOTE: those values must be no more than 4-bit.
#define DISPLAY_EXTDIGITS(INPUT0, INPUT1, INPUT2, INPUT3, INPUT4, INPUT5) \
    (INPUT0 & 0xf) << 20 | (INPUT1 & 0xf) << 16 | (INPUT2 & 0xf) << 12 |  \
    (INPUT3 & 0xf) <<  8 | (INPUT4 & 0xf) <<  4 | (INPUT5 & 0xf);

#define DISPLAY_COMMA(OFN5, OFN4, OFN3, OFN2, OFN1, OFN0)          \
    (OFN5 & 0b1) << 29 | (OFN4 & 0b1) << 28 | (OFN3 & 0b1) << 27 | \
    (OFN2 & 0b1) << 26 | (OFN1 & 0b1) << 25 | (OFN0 & 0b1) << 24

#define C_ON  0b1
#define C_OFF 0b0

// Must be kept in sync with the LUT in Decoder7Seg.vhd
#define DISPLAY_SYM_MINUS 0xa
#define DISPLAY_SYM_X     0xb
#define DISPLAY_SYM_Y     0xc
#define DISPLAY_SYM_Z     0xd
#define DISPLAY_SYM_BLANK 0xe
#define DISPLAY_SYM_C     0xf

// The display control is composed of 30-bit:
// --> [29-24] => comma lighting ;
// --> [23- 0] => 6x4-bit to control the 6x7-segments.  
#define DISPLAY_MASK 0x3fffffff

/* -*- Module Display interface -*- */
void display_calibration_start();
void display_calibration(alt_8 offset_x, alt_8 offset_y, alt_8 offset_z);
void display_acceleration(float acc, adxl345_axis_t axis);

#endif // __DISPLAY_H__
