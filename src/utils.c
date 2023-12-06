#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "../include/utils.h"

void recursive_mkdir(char *path)
{
    char *sep = strrchr(path, '/');
    if(sep != NULL) {
      *sep = 0;
      recursive_mkdir(path);
      *sep = '/';
    }
    mkdir(path, 0700);
}

char *strlwr(char *str)
{
  unsigned char *p = (unsigned char *)str;

  while (*p) {
    *p = tolower((unsigned char)*p);
    p++;
  }

  return str;
}

char toggle_lower(char ch)
{
  if (islower(ch))
    return toupper(ch);
  else
    return tolower(ch);
}

int min(int a, int b)
{
  if (b > a)
    return a;
  return b;
}

void send_notification(char *title, char *description)
{
  if (title == NULL && description == NULL)
    return;

  char * command;

  if (title == NULL)
    title = "";
  if (description == NULL)
    description = "";

  asprintf(&command, "notify-send -a potato-c -h string:x-canonical-private-synchronous:potato-c-%d  '%s' '%s'", getpid(), title,description);
  puts(command);

  (void)system((char*) command);
  free(command);
}

size_t int_length(int number)
{
  size_t output = 1;
  while (number > 1) {
    number/=10;
    output++;
  }

  return output;
}
