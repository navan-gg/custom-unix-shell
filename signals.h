#ifndef SIGNALS_H
#define SIGNALS_H

#include <sys/types.h>

//used in main.c for initial setup of signal handlers
void setup_signal_handlers(void);

// this tracks the foreground jobs
void fg_set(const pid_t *pids, int n);
int  fg_pending(void);
void fg_clear(void);

#endif 
