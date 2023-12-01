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
  int max_command_length = 512;

  #define PRINTF_TO_COMMAND snprintf((char*) command, max_command_length,"notify-send -a potato-c -h string:x-canonical-private-synchronous:potato-c-%d "
  const char *command[max_command_length];

  if (title == NULL)
    PRINTF_TO_COMMAND"'' '%s' &> /dev/null &disown",getpid(), description);
  else if (description == NULL)
    PRINTF_TO_COMMAND"'%s' '' &> /dev/null &disown",getpid(), title);
  else
    PRINTF_TO_COMMAND"'%s' '%s' &> /dev/null &disown",getpid(), title, description);

  (void)system((char*) command);
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
