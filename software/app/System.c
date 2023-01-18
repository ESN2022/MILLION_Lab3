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

// NOTE: two-level of debugging using the macro
// - ADXL345_FULL_DEBUG
// - ADXL345_DEBUG
// NOTE2: ADXL345_FULL_DEBUG => ADXL345_DEBUG
// They should be defined before the inclusion of the ADXL345 header file in 
// ADXL345.c
#include "ADXL345.h"
#include "Display.h"

// Default sensor axis
volatile adxl345_axis_t sensor_axis = ADXL345_AXIS_X;
// unit: mg/LSB
volatile float scale_factor = 0.f;

// Inlined 2^exponent (float)
#define fPOW2(exponent) (float) (0x00000001 << exponent)

static void main_irq(void* context, alt_u32 id)
{
    if (id == TIMER_0_IRQ) {
        alt_16  s_buffer_16       = 0;
        alt_u32 acc_register_addr = 0;

        if      (sensor_axis == ADXL345_AXIS_X) acc_register_addr = ADXL345_X_AXIS_0_ADDRESS;
        else if (sensor_axis == ADXL345_AXIS_Y) acc_register_addr = ADXL345_Y_AXIS_0_ADDRESS;
        else                                    acc_register_addr = ADXL345_Z_AXIS_0_ADDRESS;

        if (ADXL345_ACC10_READ(ADXL345_BASE_ADDRESS, acc_register_addr, &s_buffer_16)) {
            float y_acc = ((float)s_buffer_16)*scale_factor;
            display_acceleration(y_acc, sensor_axis);
#ifdef ADXL345_DEBUG
            alt_printf("----------------\n");
            printf("acc: %f mg\n", y_acc);
            alt_printf("----------------\n");
#endif
        }
        // reset IRQ status
        IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE, 0);
    }

    if (id == PIO_BUTTON_IRQ) {
        if (++sensor_axis > ADXL345_AXIS_Z)
            sensor_axis = ADXL345_AXIS_X;
        // IRQ status
        IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_BUTTON_BASE, 0b1);
    }
}

int main()
{
#ifdef ADXL345_DEBUG
    alt_printf("Starting on...\n");
#endif
    // Init I2C
    I2C_init(I2C_BASE_ADDRESS, ALT_CPU_FREQ, 100000);
    // Init Timer
    IOWR_ALTERA_AVALON_TIMER_PERIOD_0(TIMER_0_BASE, TIMER_0_PERIOD);
    // clear IRQ status just in case...
    IOWR_ALTERA_AVALON_TIMER_STATUS(TIMER_0_BASE, 0);
    // configure the other PIO
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_BUTTON_BASE, 0b1);
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(PIO_BUTTON_BASE, 0b1);

    // small 8-bit buffer (unsigned)
    alt_u8 u_buffer_8  = 0;

    // Configure ADXL345 in measure mode
    ADXL345_BYTE_WRITE(ADXL345_BASE_ADDRESS, ADXL345_POWER_CTL_ADDRESS, 0x08);

    // Read the Data Format register
    ADXL345_BYTE_READ(ADXL345_BASE_ADDRESS, ADXL345_DATA_FORMAT_ADDRESS, &u_buffer_8);
#ifdef ADXL345_DEBUG
    alt_printf("read data_format: %x\n", u_buffer_8);
#endif

    // g Range setting respects the following LUT:
    //  D1  | D0  |  g Range
    // -----|-----|----------
    //   0  |  0  |  +-  2g
    //   0  |  1  |  +-  4g
    //   1  |  0  |  +-  8g
    //   1  |  1  |  +- 16g
    alt_u8 g_range_setting = 0b00010 << (u_buffer_8 & 0b11);
    alt_u8 full_res        = (0b1000 & u_buffer_8) >> 3;

    // Compute the scale factor.
    // For another life, we could use that code to support the 16-bit mode.
    // 16-bit mode
    if (full_res) scale_factor = (g_range_setting*2.0*1000.0)/fPOW2(16);
    // 10-bit mode
    else scale_factor = (g_range_setting*2.0*1000.0)/fPOW2(10);

#ifdef ADXL345_DEBUG
    printf("computed scale_factor: %f mg/LSB\n", scale_factor);
#endif

    // display "C......" on the 7-segments
    display_calibration_start();
    // and wait a bit
    usleep(500000);

    // perform the auto-calibration
    alt_8 offset_x, offset_y, offset_z;
    if (ADXL345_OFFS_AUTOCBR(scale_factor, &offset_x, &offset_y, &offset_z)) 
    {
        // Writing the results in the offset registers
        ADXL345_BYTE_WRITE(ADXL345_BASE_ADDRESS, ADXL345_OFSX_ADDRESS, offset_x);
        ADXL345_BYTE_WRITE(ADXL345_BASE_ADDRESS, ADXL345_OFSY_ADDRESS, offset_y);
        ADXL345_BYTE_WRITE(ADXL345_BASE_ADDRESS, ADXL345_OFSZ_ADDRESS, offset_z);
        // display the current parameters and then wait for 1s
        display_calibration(offset_x, offset_y, offset_z);
        usleep(500000);
    }

    // Register the IRQs
    alt_irq_register(TIMER_0_IRQ   , NULL, main_irq);
    alt_irq_register(PIO_BUTTON_IRQ, NULL, main_irq);

    // Start the counter + enable interrupt + enable continuous mode
    IOWR_ALTERA_AVALON_TIMER_CONTROL(TIMER_0_BASE, 0x7);

    while (1)
    {
        usleep(1000);
    }

    return 0;
}
