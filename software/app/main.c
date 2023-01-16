#include <opencores_i2c.h>
#include <opencores_i2c_regs.h>
#include "sys/alt_stdio.h"
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include <unistd.h>
#include <alt_types.h>

// #define ADXL345_DEBUG
#include "ADXL345.h"

// TODO: when started on, configure ADXL in +-2g mode, 10-bit resolution, right justify by default
// TODO: put every alt_printf between #ifndef...#endif
adxl345_status_t ADXL345_BYTE_WRITE(alt_u32 base_address, alt_u32 reg_address, alt_u8 data) 
{
    I2C_START_WRITE    (I2C_BASE_ADDRESS, base_address);
    I2C_WRITE_BYTE     (I2C_BASE_ADDRESS, reg_address );
    I2C_WRITE_BYTE_STOP(I2C_BASE_ADDRESS, data        );

#ifdef ADXL345_DEBUG
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

#ifdef ADXL345_DEBUG
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

#ifdef ADXL345_DEBUG
    alt_printf("raw data read: %x\n", data);
#endif
    if (value) 
        *value = data;

    return ADXL345_SUCCESS;
}

// ADXL345 11-bit Acceleration Read (on the axis represented by reg_address)
// If no issues occurs, a signed 16-bit value is placed in *value variable.
adxl345_status_t ADXL345_ACC11_READ(alt_u32 base_address, alt_u32 reg_address, alt_16 *value)
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
    
#ifdef ADXL345_DEBUG
    alt_printf("(16-bit sign-extended) acc read: %x\n", data);
#endif

    return ADXL345_SUCCESS;
}

// ADXL345 Offset Calibration
adxl345_status_t ADXL345_OFF_CBR(alt_8 *offset_x, alt_8 *offset_y, alt_8 *offset_z)
{
    alt_32 acc_x = 0;
    alt_32 acc_y = 0;
    alt_32 acc_z = 0;
    alt_16 data  = 0;

    for (alt_u8 i = 0; i < ADXL345_NB_SAMPLES; i++) {
        ADXL345_ACC11_READ(ADXL345_BASE_ADDRESS, ADXL345_X_AXIS_0_ADDRESS, &data);
        acc_x += (alt_32) data;
        usleep(5);
        ADXL345_ACC11_READ(ADXL345_BASE_ADDRESS, ADXL345_Y_AXIS_0_ADDRESS, &data);
        acc_y += (alt_32) data;
        usleep(5);
        ADXL345_ACC11_READ(ADXL345_BASE_ADDRESS, ADXL345_Z_AXIS_0_ADDRESS, &data);
        acc_z += (alt_32) data;
        usleep(5);
    }

    alt_printf("data acc_x: %x\n", acc_x);
    alt_printf("data acc_y: %x\n", acc_y);
    alt_printf("data acc_z: %x\n", acc_z);

    // CHECK: signedness
    *offset_x = (((alt_8) ( acc_x/ADXL345_NB_SAMPLES)))/4;
    *offset_y = (((alt_8) ( acc_y/ADXL345_NB_SAMPLES)))/4;
    *offset_z = (alt_8) ((acc_z/ADXL345_NB_SAMPLES) + 0xff00);

    return ADXL345_SUCCESS;
}

int main() 
{
    alt_printf("Starting on...\n");

    I2C_init(I2C_BASE_ADDRESS, ALT_CPU_FREQ, 100000);
    usleep(100000);
    alt_u8 u_buffer_8  = 0;
    alt_16 s_buffer_16 = 0;

    // Configure ADXL345 in measure mode
    ADXL345_BYTE_WRITE(ADXL345_BASE_ADDRESS, ADXL345_POWER_CTL_ADDRESS, 0x08);

    if (ADXL345_BYTE_READ(ADXL345_BASE_ADDRESS, ADXL345_DATA_FORMAT_ADDRESS, &u_buffer_8))
        alt_printf("read data_format: %x\n", u_buffer_8);

    /*
        cf datasheet:        
            "There must be at least 5 us between the end of reading of the
                data registers and the start of a new read of the FIFO"
    */
    usleep(5);

    while (1) 
    {
        if (ADXL345_ACC11_READ(ADXL345_BASE_ADDRESS, ADXL345_X_AXIS_0_ADDRESS, &s_buffer_16))
            alt_printf("read x: %x\n", s_buffer_16);

        usleep(5);

        if (ADXL345_ACC11_READ(ADXL345_BASE_ADDRESS, ADXL345_Y_AXIS_0_ADDRESS, &s_buffer_16))
            alt_printf("read y: %x\n", s_buffer_16);

        usleep(5);

        if (ADXL345_ACC11_READ(ADXL345_BASE_ADDRESS, ADXL345_Z_AXIS_0_ADDRESS, &s_buffer_16))
            alt_printf("read z: %x\n", s_buffer_16);

        usleep(1000000);
    }

    return 0;
}
