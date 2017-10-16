
#include <pic.h>
#include "system.h"
#include "i2c.h"

#define I2C_WRITE   0
#define I2C_READ    1

static struct i2c_slave_struct this_slave;

#ifdef CONFIG_I2C_SLAVE_ISR
static struct i2c_slave_struct i2c_slave_data = {
    0x00,
    0x00,
    0x00,
    0x00,
};
#endif

#ifdef CONFIG_I2C_AS_MASTER
static void i2c_as_master(void)
{
   TRISB4 = 1;  // set SCL and SDA pins as inputs
   TRISB6 = 1;

   SSPCON1 = 0b00101000; 	// I2C enabled, Master mode

   SSPSTAT=0x00;
}

static void i2c_set_clk(long clk)
{
   // I2C Master mode, clock = FOSC/(4 * (SSPADD + 1))
   //                  SSPADD = FOSC / (4 * clock) - 1
   //SSPADD = 39;    	// 100Khz @ 16Mhz Fosc
   SSPADD = clk / (4 * SYS_FREQ) - 1;
}

// i2c_wait - wait for I2C transfer to finish
static void i2c_wait(void)
{
    while (SSP1IF == 0) ;
    SSP1IF = 0;
}

static int i2c_noack(void)
{
    return ACKSTAT;
}

// i2c_start - Start I2C communication
static void i2c_start(void)
{
    SEN = 1;
    i2c_wait();
}

// i2c_restart - Re-Start I2C communication
static void i2c_restart(void)
{
    RSEN = 1;
    i2c_wait();
}

// i2c_stop - Stop I2C communication
static void i2c_stop(void)
{
    PEN = 1;
    i2c_wait();
}

// i2c_write - Sends one byte of data
static void i2c_write(unsigned char data)
{
    SSPBUF = data;
    i2c_wait();
}

// i2c_address - Sends Slave Address and Read/Write mode
// mode is either I2C_WRITE or I2C_READ
static void i2c_address(unsigned char address, unsigned char mode)
{
    unsigned char l_address;

    l_address= address << 1;
    l_address += mode;
    i2c_write(l_address);
}

// i2c_read - Reads a byte from Slave device
static unsigned char i2c_read(unsigned char ack)
{
    // Read data from slave
    // ack should be 1 if there is going to be more data read
    // ack should be 0 if this is the last byte of data read
    unsigned char data;

    if (ack) {
        SSP1IF = 0;
    }
    RCEN = 1;
    i2c_wait();
    data = SSPBUF;
    if (ack) {
        ACKDT = 0;
    } else {
        ACKDT = 1;
    }
    ACKEN = 1;
    i2c_wait();
    return data;
}

int i2c_master_read(unsigned char sla, unsigned char suba,
        unsigned char* str, unsigned char no)
{
    int ret = -1;
    i2c_start();                    // send Start
    i2c_address(sla, I2C_WRITE);    // Send slave address - write operation
    if (i2c_noack()) {              // No ack
        goto out_err;
    }
    i2c_write(suba);                // Send device's register
    if (i2c_noack()) {
        goto out_err;
    }
    i2c_restart();                  // Restart
    i2c_address(sla, I2C_READ);     // Send slave address - read operation
    if (i2c_noack()) {
        goto out_err;
    }
    while (no != 1) {
        *str = i2c_read(1);
        str ++;
        no --;
    }
    *str = i2c_read(0);             // Read last byte
                                    // If more than one byte to be read, (0) should
                                    // be on last byte only
                                    // e.g.3 bytes= i2c_read(1); i2c_read(1); i2c_read(0);
out_err:
    i2c_stop();                     // send Stop
    return ret;                     // return error.
                                    // If reading more than one byte
                                    // store in an array
}

int i2c_master_write(unsigned char sla, unsigned char suba,
        unsigned char* str, unsigned char no)
{
    int ret = -1;
    unsigned char i;

    i2c_start();                    // send Start
    i2c_address(sla, I2C_WRITE);    // Send slave address - write operation
    if (i2c_noack()) {
        goto out_err;
    }
    i2c_write(suba);                // Send device's register
    if (i2c_noack()) {
        goto out_err;
    }
    for (i = 0; i < no; i ++) {
        i2c_write(*str);            // Send data
        if (i2c_noack()) {
            goto out_err;
        }
        str ++;
    }
out_err:
    i2c_stop();                     // send Stop
    return ret;
}

void i2c_master_init(long clk)
{
    i2c_as_master();
    i2c_set_clk(clk);
}
#endif

#ifdef CONFIG_I2C_AS_SLAVE
static void i2c_as_slave(unsigned char sla)
{
    TRISB4 = 1; // Set SCL and SDA pins as inputs
    TRISB6 = 1;
    WPUB4 = 0;
    WPUB6 = 0;
    nWPUEN = 1;

    SSPADD = sla << 1;

    SSPCON1 = 0b00110110; // I2C enabled, Slave mode, 7-bit address
    SSPSTAT = 0x00;

    SSP1IF = 0; // Clear MSSP interrupt request flag
    SSP1IE = 1; // Enable MSSP interrupt

    PEIE = 1;
    GIE = 1;
}

#ifdef CONFIG_I2C_SLAVE_ISR
void i2c_slave_transmit(unsigned char data)
{
    while (BF == 1) ;

    // To determine whether there is a send conflict
    while (WCOL == 1) {
        WCOL = 0; // Clear Write Collision
    }

    SSPBUF = data;

    CKP=1; // Release CLK
}

unsigned char i2c_slave_receive(void)
{
    unsigned char data;

    // To determine whether there is a conflict of reception
    while (SSPOV == 1) {
        SSPOV = 0;
    }

    data = SSPBUF;

    return data;
}

void i2c_ssp_isr(void)
{
   unsigned char stat;

    stat = SSPSTAT & 0X3D; // "00111101" Pick out effective bit in SSPSTAT
    PORTA = stat;

    switch (stat) {
    case 0x09:  // (S | BF) "00001001" Master Write operation,P=0,S=1,
                // last byte was address,buffer is full.
        //serial_putc('\n');
        //serial_putc('1');
        i2c_slave_data.bytes = 0;
        i2c_slave_data.sla = SSPBUF;
        i2c_slave_data.bytes ++;
        break;
    case 0x29:  // (S | D_nA | BF) "00101001" Msater Write operation,
                // last byte was data,S=1,P=0,buffer is full
        //serial_putc('2');
        if (i2c_slave_data.bytes <= 1) {
            i2c_slave_data.suba = i2c_slave_receive(); //Get data from bus
        } else {
            i2c_slave_data.data = i2c_slave_receive(); //Get data from bus
        }
        i2c_slave_data.bytes ++;
        break;
    case 0x31:  // (P | D_nA | BF) "00110001" Master  Write operation,
                // last byte was data,S=0,P=1,buffer is full
        //serial_putc('3');
        i2c_slave_data.data = i2c_slave_receive(); //Get data from bus
        i2c_slave_data.bytes ++;
        break;
    case 0x0C:  // (R_nW | D_nA) "00001100" Master Read operation,
                // last byte was address,S=1,P=0,buffer is empty.
        //serial_putc('4');
        i2c_slave_transmit(0x55); // e.g. Send data 0x55
        break;
    case 0x2C:  // (S | R_nW | D_nA) "00101100" Master Read operation,
                // last byte was data,S=1,P=0,buffer is empty.
        //serial_putc('5');
        i2c_slave_data.data = i2c_slave_receive(); //Receive the master command,and clear buffer.
        i2c_slave_data.bytes ++;
        break;
    default:
        //serial_putc('?');
        i2c_slave_data.bytes = 0;
        break;
    }
}
#endif

void i2c_slave_init(unsigned char sla)
{
    i2c_as_slave(sla);
    serial_puts("(I): [I2C] Init as slave mode.\n");
    serial_puts("(I): [I2C] Address: ");
    serial_put_hex(sla);
    serial_putc('\n');
}
#endif

void i2c_isr(void)
{
    volatile unsigned char dummy = 0;

    SSP1CON1bits.CKP = 0;       // Hold the clock low while processing
    // Check for overflow or collision errors and reset as required
    if ((SSP1CON1bits.SSPOV) || (SSP1CON1bits.WCOL)) {
        //serial_putc('?');
        dummy = SSP1BUF;        // Read the previous value to clear the buffer
        SSP1CON1bits.SSPOV = 0; // Clear the overflow flag
        SSP1CON1bits.WCOL = 0;  // Clear the collision bit
        SSP1CON1bits.CKP = 1;   // Release the clock
        return;
    }

    // master read
    if (SSPSTATbits.R_nW) {
        //serial_putc('R');
        //dummy = dummy; //Do something

        if (SSPSTATbits.D_nA == 0) {
            //serial_putc('1');
            // New Address
            // First byte that is being read:
            SSPBUF = dummy;
        } else {
            //serial_putc('2');
            // New data
            // Oncoming byte that are being read:
            SSPBUF = dummy;
        }

        //this_slave.bytes = 0;
    }

    // master write
    if (!SSPSTATbits.R_nW) {
        // We received a new address
        if (SSPSTATbits.D_nA == 0) {
            //serial_putc('3');
            // reset data statemachine here.
            dummy = SSPBUF;

            this_slave.sla = dummy;
            this_slave.bytes = 0;
        } else {
            //serial_putc('4');
            // read dummy byte
            dummy = SSPBUF;

            this_slave.bytes++;
            if (this_slave.bytes == 1) {
                this_slave.suba = dummy;
            } else if (this_slave.bytes == 2){
                this_slave.data = dummy;
                //Do something
            }
        }
    }

    SSP1CON1bits.CKP = 1;   // Release the clock
}