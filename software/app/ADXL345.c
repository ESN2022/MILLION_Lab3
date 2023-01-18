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

// #define ADXL345_FULL_DEBUG
// #define ADXL345_DEBUG
#include "ADXL345.h"

adxl345_status_t ADXL345_BYTE_WRITE(alt_u32 base_address, alt_u32 reg_address, alt_u8 data)
{
    I2C_START_WRITE    (I2C_BASE_ADDRESS, base_address);
    I2C_WRITE_BYTE     (I2C_BASE_ADDRESS, reg_address );
    I2C_WRITE_BYTE_STOP(I2C_BASE_ADDRESS, data        );

#ifdef ADXL345_FULL_DEBUG
    alt_printf("sucessfully write data %x at %x\n", data, reg_address);
#endif
    return ADXL345_SUCCESS;
}

// ADXL345 Single-Byte Read
adxl345_status_t ADXL345_BYTE_READ(alt_u32 base_address, alt_u32 reg_address, alt_u8 *value)
{
    int value_read = 0;
    alt_u8 data    = 0;

    I2C_START_WRITE(I2C_BASE_ADDRESS, base_address);
    I2C_WRITE_BYTE (I2C_BASE_ADDRESS, reg_address );
    I2C_START_READ (I2C_BASE_ADDRESS, base_address);

    value_read = I2C_READ_BYTE_NACK_STOP(I2C_BASE_ADDRESS);
    data = (0xff & value_read);

#ifdef ADXL345_FULL_DEBUG
    alt_printf("raw data read: %x\n", data);
#endif
    if (value)
        *value = data;

    return ADXL345_SUCCESS;
}

// ADXL345 2-Byte Read in Burst Mode
// It performs a 2-byte consecutive read, on 2 adjacent registers:
//                 16-bit
//  <====================================> 
// [read(reg_address+1), read(reg_address)]
//  MSB                                LSB
adxl345_status_t ADXL345_2BYTE_READ_BURST(alt_u32 base_address, alt_u32 reg_address, alt_u16 *value)
{
    int value_read = 0;
    alt_u16 data   = 0;

    I2C_START_WRITE(I2C_BASE_ADDRESS, base_address);
    I2C_WRITE_BYTE (I2C_BASE_ADDRESS, reg_address );
    I2C_START_READ (I2C_BASE_ADDRESS, base_address);

    value_read = I2C_READ_BYTE_ACK(I2C_BASE_ADDRESS);
    data = (0xff & value_read);
    value_read = I2C_READ_BYTE_NACK_STOP(I2C_BASE_ADDRESS);
    data = ((0xff & value_read) << 8) | data;

#ifdef ADXL345_FULL_DEBUG
    alt_printf("raw data read: %x\n", data);
#endif
    if (value) 
        *value = data;

    return ADXL345_SUCCESS;
}

// ADXL345 10-bit Acceleration Read (on the axis represented by reg_address)
// If no issues occurs, a signed 16-bit value is placed in *value variable.
adxl345_status_t ADXL345_ACC10_READ(alt_u32 base_address, alt_u32 reg_address, alt_16 *value)
{
    // Raw data buffer
    alt_u16 data = 0;
    
    if (reg_address < ADXL345_X_AXIS_0_ADDRESS || reg_address >
        ADXL345_Z_AXIS_1_ADDRESS)
        return ADXL345_ERROR;
    
    if (ADXL345_2BYTE_READ_BURST(base_address, reg_address, &data) != ADXL345_SUCCESS)
        return ADXL345_ERROR;
    
    // 10-bit resolution = 9-bit of values + 1 sign bit
    // => Perform the sign-extension based on the value of the 10th bit
    if (0x200 & data) *value = 0xfc00 | (data & 0x3ff);
    else              *value = 0x0000 | (data & 0x3ff);
    
#ifdef ADXL345_FULL_DEBUG
    alt_printf("(16-bit sign-extended) acc read: %x\n", data);
#endif

    return ADXL345_SUCCESS;
}

// ADXL345 Offset Auto-Calibration
adxl345_status_t ADXL345_OFFS_AUTOCBR(float scale_factor_read, alt_8 *offset_x, alt_8 *offset_y, alt_8 *offset_z)
{
    double acc_x = 0;
    double acc_y = 0;
    double acc_z = 0;
    alt_16 data  = 0;

    for (alt_u16 i = 0; i < (alt_u16) ADXL345_NB_SAMPLES; i++) {
        ADXL345_ACC10_READ(ADXL345_BASE_ADDRESS, ADXL345_X_AXIS_0_ADDRESS, &data);
        acc_x += (double) data;
        usleep(5);
        ADXL345_ACC10_READ(ADXL345_BASE_ADDRESS, ADXL345_Y_AXIS_0_ADDRESS, &data);
        acc_y += (double) data;
        usleep(5);
        ADXL345_ACC10_READ(ADXL345_BASE_ADDRESS, ADXL345_Z_AXIS_0_ADDRESS, &data);
        acc_z += (double) data;
        usleep(5);
    }

    float mean_x = (acc_x/(float)ADXL345_NB_SAMPLES)*scale_factor_read;
    float mean_y = (acc_y/(float)ADXL345_NB_SAMPLES)*scale_factor_read;
    float mean_z = (acc_z/(float)ADXL345_NB_SAMPLES)*scale_factor_read;

    float offset_xf =    0.f - mean_x;
    float offset_yf =    0.f - mean_y;
    // 1000.f is it always the case ?
    float offset_zf = 1000.f - mean_z;

    // if at least one of these pointers is NULL... 
    if (!(offset_x && offset_y && offset_z))
        return ADXL345_ERROR;

    *offset_x = (alt_8) (offset_xf / ADXL345_SF_OFFSET_REG);
    *offset_y = (alt_8) (offset_yf / ADXL345_SF_OFFSET_REG);
    *offset_z = (alt_8) (offset_zf / ADXL345_SF_OFFSET_REG);

#ifdef ADXL345_FULL_DEBUG
    alt_printf("offset_x: %x\n", *offset_x);
    alt_printf("offset_y: %x\n", *offset_y);
    alt_printf("offset_z: %x\n", *offset_z);
#endif

    return ADXL345_SUCCESS;
}
