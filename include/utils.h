#ifndef UTILS_H__
#define UTILS_H__
#include <stddef.h>
void send_notification(char *title, char *description);
size_t int_length(int);
int min(int a, int b);
#define LENGTH(X) (sizeof X / sizeof X[0])
#endif
