#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "../include/utils.h"
#include "../config.h"

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

struct format_notif_args {
  notif_t notif;
  int pid;
};
static char *
format_notification(void *arguments, char format_char)
{
  struct format_notif_args *args = arguments;
  char *str;
  switch (format_char) {
    case 't':
      asprintf(&str, "%s", args->notif.title);
    break;
    case 'b':
      asprintf(&str, "%s", args->notif.body);
    break;
    case 'p':
      asprintf(&str, "%d", args->pid);
    break;
    default:
      errno = 1;
      perror("Notification format character not defined");
      exit(1);
    break;
  }
  return str;
}

void send_notification(notif_t notif)
{
  if (notif.title == NULL && notif.body == NULL)
    return;

  if (notif.title == NULL)
    notif.title = "";
  if (notif.body == NULL)
    notif.body = "";

  struct format_notif_args args = {.notif = notif, .pid = getpid()};
  char * command = resolve_format(notification_format, format_notification, &args);

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

char* resolve_format(char const* format, char* handler(void*, char), void* args)
{
  int format_length = strlen(format);
  char* output = malloc((format_length + 1) * sizeof(char));
  int output_index = 0;

  for (char const* fmt_ptr = format; *fmt_ptr; fmt_ptr++) {
    if (fmt_ptr[0] == '%') {
      if (fmt_ptr[1] != '%') {
        char* string_formatted = handler(args, fmt_ptr[1]);
        int string_length = strlen(string_formatted);
        memcpy(output + output_index, string_formatted, string_length);
        output_index += string_length;
        free(string_formatted);
      } else {
        output[output_index++] = '%';
      }
      fmt_ptr++;
    } else {
      output[output_index++] = *fmt_ptr;
    }
  }

  output[output_index] = '\0';
  return output;
}
