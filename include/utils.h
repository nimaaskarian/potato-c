#ifndef UTILS_H__
#define UTILS_H__
#include <stddef.h>
typedef struct{
  const char * title;
  const char * body;
} notif_t;
void recursive_mkdir(char *path);
void send_notification(notif_t notif);
size_t int_length(int);
int min(int a, int b);
char toggle_lower(char ch);
char * resolve_format(char const *format, char * handler(void *, char), void * args);
#define LENGTH(X) (sizeof X / sizeof X[0])
#endif
