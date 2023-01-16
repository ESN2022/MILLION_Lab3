#include <opencores_i2c.h>
#include <opencores_i2c_regs.h>
#include "sys/alt_stdio.h"
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include <unistd.h>

// TODO: configure in 16g mode

int main() {
    I2C_init(OPENCORES_I2C_0_BASE, ALT_CPU_FREQ, 100000);
    usleep(100000);
    int data;
    alt_u8 issou;

    while (1) {
        alt_u8 ack = I2C_start(OPENCORES_I2C_0_BASE, 0x53, 0b0);
        alt_printf("start status: %x\n", ack);
        usleep(10);
        I2C_write(OPENCORES_I2C_0_BASE,0x36,0);
        usleep(10);
        ack = I2C_start(OPENCORES_I2C_0_BASE, 0x53, 0b1);
        usleep(10);
        
        if (ack == 0) {
            // read the input register and don't send stop
            data = I2C_read(0x36, 0b0);
            issou = 0xff & data;
            alt_printf("read z0: %x\n", issou);
        }
        usleep(1000000);
    }

    return 0;
}
