#include "lib-header/cmos.h"
#include "lib-header/portio.h"

void cmos_read_rtc(struct time *t)
{
    // Read the CMOS time registers
    out(CMOS_ADDRESS_PORT, 0x00);
    t->second = in(CMOS_DATA_PORT);
    out(CMOS_ADDRESS_PORT, 0x02);
    t->minute = in(CMOS_DATA_PORT);
    out(CMOS_ADDRESS_PORT, 0x04);
    t->hour = in(CMOS_DATA_PORT);
    out(CMOS_ADDRESS_PORT, 0x07);
    t->day = in(CMOS_DATA_PORT);
    out(CMOS_ADDRESS_PORT, 0x08);
    t->month = in(CMOS_DATA_PORT);
    out(CMOS_ADDRESS_PORT, 0x09);
    t->year = in(CMOS_DATA_PORT);
    out(CMOS_ADDRESS_PORT, 0x32);
    t->year |= in(CMOS_DATA_PORT) << 8;

    // Convert BCD values to binary
    t->second = ((t->second & 0xF0) >> 4) * 10 + (t->second & 0x0F);
    t->minute = ((t->minute & 0xF0) >> 4) * 10 + (t->minute & 0x0F);

    t->hour = ((t->hour & 0xF0) >> 4) * 10 + (t->hour & 0x0F);
    if ((t->hour & 0x80) != 0) // if the high bit is set
    {
        // convert to 24-hour format by subtracting 12 from the hours
        t->hour = ((t->hour & 0x7F) + 12) % 24;
    }
    
    t->day = ((t->day & 0xF0) >> 4) * 10 + (t->day & 0x0F);
    t->month = ((t->month & 0xF0) >> 4) * 10 + (t->month & 0x0F);
    t->year = ((t->year & 0xF0) >> 4) * 10 + (t->year & 0x0F);
}