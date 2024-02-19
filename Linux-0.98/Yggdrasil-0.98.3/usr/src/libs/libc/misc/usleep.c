#include <stdlib.h>
#include <sys/time.h>

void
usleep(unsigned long usec)
{
        struct timeval timeout;

        timeout.tv_sec = usec / 1000000;
        timeout.tv_usec = usec - 1000000 * timeout.tv_sec;
        select(1, NULL, NULL, NULL, &timeout);
}
