#ifndef CMOS_H
#define CMOS_H

#include <asm/io.h>

#define CMOS_READ(addr) ({\
outb_p(addr | 0x80, 0x70);\
inb_p(0x71);\
})

#define RTC_SECOND		0
#define RTC_SECOND_ALRAM	1
#define RTC_MINUTE		2
#define RTC_MINUTE_ALRAM	3
#define RTC_HOUR		4
#define RTC_HOUR_ALRAM		5
#define RTC_DAY_OF_WEEK		6
#define RTC_DAY_OF_MONTH	7
#define RTC_MONTH		8
#define RTC_YEAR		9

#define RTC_REG_A		10
#define RTC_REG_B		11
#define RTC_REG_C		12
#define RTC_REG_D		13

#define RTC_CENTURY		0x32

#ifndef BCD_TO_BIN
#define BCD_TO_BIN(val) (val = (val & 0x0f) + ((val >> 4) * 10))
#endif

#ifndef BIN_TOBCD
#define BIN_TO_BCD(val) (val = ((val / 10) << 4) + (val % 10))
#endif

#endif
