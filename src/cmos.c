#include "lib-header/cmos.h"

void get_cmos_t(struct time *t)
{
    // Read the CMOS time registers
    outb(CMOS_ADDRESS_PORT, 0x00);
    t->second = inb(CMOS_DATA_PORT);
    outb(CMOS_ADDRESS_PORT, 0x02);
    t->minute = inb(CMOS_DATA_PORT);
    outb(CMOS_ADDRESS_PORT, 0x04);
    t->hour = inb(CMOS_DATA_PORT);
    outb(CMOS_ADDRESS_PORT, 0x07);
    t->day = inb(CMOS_DATA_PORT);
    outb(CMOS_ADDRESS_PORT, 0x08);
    t->month = inb(CMOS_DATA_PORT);
    outb(CMOS_ADDRESS_PORT, 0x09);
    t->year = inb(CMOS_DATA_PORT);
    outb(CMOS_ADDRESS_PORT, 0x32);
    t->year |= inb(CMOS_DATA_PORT) << 8;
}