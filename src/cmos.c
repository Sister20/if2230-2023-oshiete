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
}