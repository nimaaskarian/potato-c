#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "utils.h"


void send_notification(char *title, char *description)
{
  int max_command_length = 512;

  const char *command[max_command_length];

  snprintf((char*) command, max_command_length, "notify-send -a potato-c -h string:x-canonical-private-synchronous:potato-c-%d \"%s\" \"%s\" ",getpid(), title, description);
  (void)system((char*) command);
}

