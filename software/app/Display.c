#include <alt_types.h>
#include <math.h>
#include <opencores_i2c.h>
#include <opencores_i2c_regs.h>
#include <stdio.h>
#include <unistd.h>

#include "altera_avalon_pio_regs.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_timer.h"
#include "system.h"
#include "sys/alt_irq.h"
#include "sys/alt_stdio.h"

#include "Display.h"

// (Slightly) Adjusted from https://programmingpraxis.com/2018/07/31/double-dabble/
alt_u32 double_dabble(alt_u32 n) {
    alt_u64 bcd = n;
    for (unsigned i = 0; i < 32; i++) {
        for (unsigned j = 0; j < 32; j += 4) {
            if (((bcd >> (32 + j)) & 0x0000000F) > 4)
                bcd += ((alt_u64)3 << (32 + j));
        }
        bcd <<= 1;
    }
    bcd >>= 32;
    return (alt_u32) bcd;
}

void display_calibration_start()
{
    alt_u32 display = DISPLAY_COMMA(C_ON, C_ON, C_ON, C_ON, C_ON, C_ON) |
                      // "C" means calibration...
                      DISPLAY_EXTDIGITS(DISPLAY_SYM_C,
                                        DISPLAY_SYM_C,
                                        DISPLAY_SYM_C, 
                                        DISPLAY_SYM_C, 
                                        DISPLAY_SYM_C, 
                                        DISPLAY_SYM_C);

    IOWR_ALTERA_AVALON_PIO_DATA(PIO_DISPLAYERS_BASE, display & DISPLAY_MASK);
}

void display_calibration(alt_8 offset_x, alt_8 offset_y, alt_8 offset_z)
{
    alt_u8 bcd_x = 0;
    alt_u8 dis_off_x = DISPLAY_SYM_BLANK;
    alt_u8 bcd_y = 0;
    alt_u8 dis_off_y = DISPLAY_SYM_BLANK;
    alt_u8 bcd_z = 0;
    alt_u8 dis_off_z = DISPLAY_SYM_BLANK;

    bcd_x = (alt_u8) double_dabble((alt_u32) offset_x);
    if (offset_x < 0) {
        offset_x = -offset_x;
        dis_off_x = DISPLAY_SYM_MINUS;
    }

    bcd_y = (alt_u8) double_dabble((alt_u32) offset_y);
    if (offset_y < 0) {
        offset_y = -offset_y;
        dis_off_y = DISPLAY_SYM_MINUS;
    }

    bcd_z = (alt_u8) double_dabble((alt_u32) offset_z);
    if (offset_z < 0) {
        offset_z = -offset_z;
        dis_off_z = DISPLAY_SYM_MINUS;
    }

    alt_u32 display = DISPLAY_COMMA(C_OFF, C_ON, C_OFF, C_ON, C_OFF, C_ON) |
                      DISPLAY_EXTDIGITS(dis_off_x, bcd_x, 
                                        dis_off_y, bcd_y, 
                                        dis_off_z, bcd_z);

    IOWR_ALTERA_AVALON_PIO_DATA(PIO_DISPLAYERS_BASE, display & DISPLAY_MASK);
}

void display_acceleration(float acc, adxl345_axis_t axis)
{
    alt_u32 display = 0;
    if (acc < 0)
        acc  = -acc;

    double integral   = 0.;
    double fractional = modf(acc, &integral) * 10.;

    alt_u32 integral_bcd   = double_dabble((alt_u32) integral  ) & 0xfff;
    alt_u32 fractional_bcd = double_dabble((alt_u32) fractional) & 0xf;

    alt_u8 display_axis = 0;
    if (axis == ADXL345_AXIS_X) display_axis = DISPLAY_SYM_X;
    if (axis == ADXL345_AXIS_Y) display_axis = DISPLAY_SYM_Y;
    if (axis == ADXL345_AXIS_Z) display_axis = DISPLAY_SYM_Z;

    alt_u8 fourth_digit = 0;

    // putting everything into position
    display =  DISPLAY_COMMA(C_OFF, C_OFF, C_OFF, C_OFF, C_ON, C_OFF) |
               DISPLAY_EXTDIGITS(display_axis,
                                 // if there is no sign to print, add a 4th digit
                                 // iff the 4th digit is > 0
                                 (acc < 0) ? DISPLAY_SYM_MINUS : 
                                             ((fourth_digit = (integral_bcd & 0xf000) >> 12) > 0) ?
                                               fourth_digit : DISPLAY_SYM_BLANK,
                                 (integral_bcd & 0xf00) >> 8, 
                                 (integral_bcd & 0xf0 ) >> 4,
                                 (integral_bcd & 0xf  )     ,
                                 fractional_bcd);
    
    IOWR_ALTERA_AVALON_PIO_DATA(PIO_DISPLAYERS_BASE, display & 0x3fffffff);
}
