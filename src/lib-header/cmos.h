#ifndef _CMOS_H
#define _CMOS_H

#include "stdtype.h"

// Define the CMOS ports
#define CMOS_ADDRESS_PORT 0x70
#define CMOS_DATA_PORT 0x71

struct time
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

/**
 * cmos_read_rtc - Read the current date and time from the CMOS RTC
 *
 * @param year      Output parameter for the current year (in BCD format)
 * @param month     Output parameter for the current month (in BCD format)
 * @param day       Output parameter for the current day of the month (in BCD format)
 * @param hour      Output parameter for the current hour (in BCD format)
 * @param minute    Output parameter for the current minute (in BCD format)
 * @param second    Output parameter for the current second (in BCD format)
 */
void cmos_read_rtc(time *t);

#endif
