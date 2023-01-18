#ifndef __ADXL345_H__
#define __ADXL345_H__

// ADXL345_FULL_DEBUG => ADXL345_DEBUG
#ifdef  ADXL345_FULL_DEBUG
#define ADXL345_DEBUG
#endif // ADXL345_FULL_DEBUG

typedef enum {
    // Force ADXL345_ERROR to 0 as we want if statement to fail if this code
    // is returned. 
    ADXL345_ERROR = 0,
    ADXL345_SUCCESS
} adxl345_status_t;

typedef enum {
    ADXL345_AXIS_X,
    ADXL345_AXIS_Y,
    ADXL345_AXIS_Z
} adxl345_axis_t;

#define I2C_BASE_ADDRESS            OPENCORES_I2C_0_BASE
#define ADXL345_NB_SAMPLES          300.0
// As ALT_ADDRESS is wired to HIGH
#define ADXL345_BASE_ADDRESS        0x53

// A few registers of the ADXL345 we're going to use:
#define ADXL345_OFSX_ADDRESS        0x1E
#define ADXL345_OFSY_ADDRESS        0x1F
#define ADXL345_OFSZ_ADDRESS        0x20
#define ADXL345_POWER_CTL_ADDRESS   0x2D
#define ADXL345_X_AXIS_0_ADDRESS    0x32
#define ADXL345_X_AXIS_1_ADDRESS    0x33
#define ADXL345_Y_AXIS_0_ADDRESS    0x34
#define ADXL345_Y_AXIS_1_ADDRESS    0x35
#define ADXL345_Z_AXIS_0_ADDRESS    0x36
#define ADXL345_Z_AXIS_1_ADDRESS    0x37
#define ADXL345_DATA_FORMAT_ADDRESS 0x31

// Constant Scale factor of the Offset registers => 15.6 mg/LSB
#define ADXL345_SF_OFFSET_REG 15.6

#ifndef ADXL345_FULL_DEBUG
#define I2C_START_WRITE(BASE_ADDRESS, DEV_ADRESS) \
    if (I2C_start(BASE_ADDRESS, DEV_ADRESS, 0b0)  != I2C_ACK) return ADXL345_ERROR;
#define I2C_START_READ(BASE_ADDRESS, DEV_ADRESS) \
    if (I2C_start(BASE_ADDRESS, DEV_ADRESS, 0b1)  != I2C_ACK) return ADXL345_ERROR;
#define I2C_WRITE_BYTE(BASE_ADDRESS, REG_ADDRESS) \
    if (I2C_write(BASE_ADDRESS, REG_ADDRESS, 0b0) != I2C_ACK) return ADXL345_ERROR;
#define I2C_WRITE_BYTE_STOP(BASE_ADDRESS, REG_ADDRESS) \
    if (I2C_write(BASE_ADDRESS, REG_ADDRESS, 0b1) != I2C_ACK) return ADXL345_ERROR;

#else

#define I2C_START_WRITE(BASE_ADDRESS, DEV_ADRESS)            \
    {                                                        \
        int ack = I2C_start(BASE_ADDRESS, DEV_ADRESS, 0b0);  \
        alt_printf("start write status: %x\n", ack);         \
        if (ack != I2C_ACK) {  return ADXL345_ERROR; }       \
    }
#define I2C_START_READ(BASE_ADDRESS, DEV_ADRESS)             \
    {                                                        \
        int ack = I2C_start(BASE_ADDRESS, DEV_ADRESS, 0b1);  \
        alt_printf("start read status: %x\n", ack);          \
        if (ack != I2C_ACK) {  return ADXL345_ERROR; }       \
    }
#define I2C_WRITE_BYTE(BASE_ADDRESS, DATA)                   \
    {                                                        \
        int ack = I2C_write(BASE_ADDRESS, DATA, 0b0);        \
        alt_printf("write data %x status: %x\n", DATA, ack); \
        if (ack != I2C_ACK) {  return ADXL345_ERROR; }       \
    }
#define I2C_WRITE_BYTE_STOP(BASE_ADDRESS, DATA)              \
    {                                                        \
        int ack = I2C_write(BASE_ADDRESS, DATA, 0b1);        \
        alt_printf("write data %x status: %x\n", DATA, ack); \
        if (ack != I2C_ACK) {  return ADXL345_ERROR; }       \
    }

#endif

#define I2C_READ_BYTE_ACK(BASE_ADDRESS) \
    I2C_read(BASE_ADDRESS, 0b0);

#define I2C_READ_BYTE_NACK_STOP(BASE_ADDRESS) \
    I2C_read(BASE_ADDRESS, 0b1);


/* -*- Module ADXL345 interface -*- */

// ADXL345 Single-Byte Write
adxl345_status_t ADXL345_BYTE_WRITE(alt_u32 base_address, 
                                    alt_u32 reg_address, 
                                    alt_u8 data);

// ADXL345 Single-Byte Read
adxl345_status_t ADXL345_BYTE_READ(alt_u32 base_address, 
                                   alt_u32 reg_address, 
                                   alt_u8 *value);

// ADXL345 2-Byte Read in Burst Mode
// It performs a 2-byte consecutive read, on 2 adjacent registers:
//                 16-bit
//  <====================================> 
// [read(reg_address+1), read(reg_address)]
//  MSB                                LSB
adxl345_status_t ADXL345_2BYTE_READ_BURST(alt_u32 base_address, 
                                          alt_u32 reg_address, 
                                          alt_u16 *value);

// ADXL345 10-bit Acceleration Read (on the axis represented by reg_address)
// If no issues occurs, a signed 16-bit value is placed in the *value variable.
adxl345_status_t ADXL345_ACC10_READ(alt_u32 base_address, 
                                    alt_u32 reg_address, 
                                    alt_16 *value);

// ADXL345 Offset Auto-Calibration
adxl345_status_t ADXL345_OFFS_AUTOCBR(float scale_factor_read, 
                                      alt_8 *offset_x, 
                                      alt_8 *offset_y, 
                                      alt_8 *offset_z);

#endif // __ADXL345_H__
