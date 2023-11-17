#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/utils.h"

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
