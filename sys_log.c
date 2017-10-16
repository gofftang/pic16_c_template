/*
 * File:
 * Author: gofftang
 *
 * Created on 2016/7/4, 21:00
 */

#include <xc.h>
#include "uart.h"

#define SYS_LOGE   0
#define SYS_LOGW   1
#define SYS_LOGI   2
#define SYS_LOGD   3

static unsigned char this_log_level = SYS_LOGD;

void sys_log_d(const char* tag, const char* fmt)
{
    if (this_log_level >= SYS_LOGD) {
        serial_puts("(D): ");
        serial_puts(tag);
        serial_putc(' ');
        serial_puts(fmt);
        serial_putc('\n');
    }
}

void sys_log_i(const char* tag, const char* fmt)
{
    if (this_log_level >= SYS_LOGI) {
        serial_puts("(I): ");
        serial_puts(tag);
        serial_putc(' ');
        serial_puts(fmt);
        serial_putc('\n');
    }
}

void sys_log_w(const char* tag, const char* fmt)
{
    if (this_log_level >= SYS_LOGW) {
        serial_puts("(W): ");
        serial_puts(tag);
        serial_putc(' ');
        serial_puts(fmt);
        serial_putc('\n');
    }
}

void sys_log_e(const char* tag, const char* fmt)
{
    serial_puts("(E): ");
    serial_puts(tag);
    serial_putc(' ');
    serial_puts(fmt);
    serial_putc('\n');
}
