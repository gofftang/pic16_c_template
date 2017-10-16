//include..
#include "adc.h"
#include "uart.h"

/******************************************************************************/
/* System Level #define Macros                                                */
/******************************************************************************/
//#define CONFIG_DEBUG

//#define CONFIG_UART

#define CONFIG_I2C_AS_SLAVE
//#define CONFIG_I2C_AS_MASTER
//#define CONFIG_I2C_SLAVE_ISR

/* TODO Define system operating frequency */

/* Microcontroller MIPs (FCY) */
#define SYS_FREQ        16000000L //16MHz
#define FCY             (SYS_FREQ / 4)
#define _XTAL_FREQ      SYS_FREQ    //__delay_ms use the marco '_XTAL_FREQ'

#define SYS_UART_RATE   9600

/******************************************************************************/
/* System Function Prototypes                                                 */
/******************************************************************************/
void sys_log_d(const char* tag, const char* fmt);
void sys_log_i(const char* tag, const char* fmt);
void sys_log_w(const char* tag, const char* fmt);
void sys_log_e(const char* tag, const char* fmt);

/* Custom oscillator configuration funtions, reset source evaluation
functions, and other non-peripheral microcontroller initialization functions
go here. */

void ConfigureOscillator(void); /* Handles clock switching/osc initialization */
