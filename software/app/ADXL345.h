#ifndef __ADXL345_H__
#define __ADXL345_H__

typedef enum {
    ADXL345_ERROR = 0,
    ADXL345_SUCCESS
} adxl345_status_t;

#define I2C_BASE_ADDRESS            OPENCORES_I2C_0_BASE
#define ADXL345_NB_SAMPLES          10
#define ADXL345_BASE_ADDRESS        0x53

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

#ifndef ADXL345_DEBUG
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

#endif
