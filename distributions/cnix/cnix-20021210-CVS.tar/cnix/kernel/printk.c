#include <stdarg.h>

static char buf[1024];

int printk(const char * fmt, ...)
{
	int i;
	va_list args;

	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);

	puts(buf);

	return i;
}

int panic(const char * fmt, ...)
{
	int i;
	va_list args;

	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);

	puts(buf);

	for(;;);

	return i;
}
