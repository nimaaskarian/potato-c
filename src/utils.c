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
  char * title;
  char * description;
  int pid;
};
static char *
format_notification(void *arguments, char format_char)
{
  struct format_notif_args *args = arguments;
  char *str;
  switch (format_char) {
    case 't':
      asprintf(&str, "%s", args->title);
    break;
    case 'd':
      asprintf(&str, "%s", args->description);
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

void send_notification(char *title, char *description)
{
  if (title == NULL && description == NULL)
    return;

  if (title == NULL)
    title = "";
  if (description == NULL)
    description = "";

  struct format_notif_args args = {.title = title, .description = description, .pid = getpid()};
  char * command = resolve_format(notification_basecmd, format_notification, &args);

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

char * resolve_format(char const *format, char * handler(void *, char), void * args)
{
  char * output = "";
  int output_index = 0;
  // variable of format current pointer is fmt_ptr
  for (char const *fmt_ptr = format; *fmt_ptr; fmt_ptr++) {
    if (fmt_ptr[0] == '%') {
      char *string_formated;
      if (fmt_ptr[1] != '%')
         string_formated = handler(args, fmt_ptr[1]);
      else
        asprintf(&string_formated, "%%");
      output_index = asprintf(&output,"%s%s", output,string_formated);
      free(string_formated);
      // add once more, cause the next char was our format char
      fmt_ptr++;
    } else {
      output_index = asprintf(&output,"%s%c", output, fmt_ptr[0]);
    }
  }
  return output;
}
