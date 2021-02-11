#include "helpers.h"
#include <signal.h>
#include <time.h>
#include <stdio.h>

int set_signal_handler(int signo, void (*handler)(int)) {
    struct sigaction sa;
    sa.sa_handler = handler;  
    sigemptyset(&sa.sa_mask);  
    sa.sa_flags = SA_RESTART;       
    return sigaction(signo, &sa, NULL);
}