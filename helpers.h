#ifndef _HELPERS_H
#define _HELPERS_H

int set_signal_handler(int signo, void (*handler)(int));
void afficher_carte(int* tab);

#endif