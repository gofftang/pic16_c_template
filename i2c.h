#ifndef I2C_H
#define	I2C_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "system.h"

#ifdef CONFIG_I2C_AS_MASTER
int i2c_master_read(unsigned char sla, unsigned char suba,
                        unsigned char* str, unsigned char no);
int i2c_master_write(unsigned char sla, unsigned char suba,
                        unsigned char* str, unsigned char no);
void i2c_master_init(long clk);
#endif

#ifdef CONFIG_I2C_AS_SLAVE
void i2c_slave_init(unsigned char sla);
void i2c_slave_transmit(unsigned char data);
unsigned char i2c_slave_receive(void);
//  byte0   byte1   byte2   byte3   ...
//  sla     suba    data0   data1   ...
struct i2c_slave_struct {
    unsigned char bytes; // receive bytes;
    unsigned char sla;
    unsigned char suba;
    unsigned char data; // only support 1byte access right now
};
#endif

#ifdef	__cplusplus
}
#endif

#endif /* I2C_H */

