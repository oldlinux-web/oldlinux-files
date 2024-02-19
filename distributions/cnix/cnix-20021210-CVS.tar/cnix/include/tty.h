#ifndef TTY_H
#define TTY_H

#define SIZE 64 /*  the power of 2 ,so mod is replaced by head &(SIZE - 1) */
struct tty_queue {
		  long head;
		  long tail;
		  char buf[SIZE];
};

#endif
