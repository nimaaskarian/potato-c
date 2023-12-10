#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../config.h"
#include "../include/timer.h"
#include "../include/utils.h"

void Timer_pause(Timer *restrict timer)
{
  timer->paused = true;
}

void Timer_unpause(Timer *restrict timer)
{
  timer->paused = false;
}

void Timer_toggle_pause(Timer *restrict timer)
{
  if (timer->paused)
    Timer_unpause(timer);
  else
    Timer_pause(timer);
}

// This method has been designed in a way to just be 
// put inside a loop and just work for you as your timer.
// You can use `Timer_print_time_left` function afterwards
void Timer_sleep_reduce_second(Timer *restrict timer)
{
  sleep(1);
  if (!timer->paused) {
    timer->seconds--;
  }
  if (!timer->seconds) {
    Timer_cycle_type(timer);
    Timer_set_seconds_based_on_type(timer);
  }
}

void Timer_set_seconds_based_on_type(Timer *restrict timer)
{
  float minutes;
  switch (timer->type) {
    case POMODORO_TYPE:
      minutes = pomodoro_minutes;
    break;
    case SHORT_BREAK_TYPE:
      minutes = short_break_minutes;
    break;
    case LONG_BREAK_TYPE:
      minutes = long_break_minutes;
    break;
  }
  timer->seconds = minutes*SECONDS_IN_MINUTES;
}

void Timer_cycle_type(Timer *restrict timer)
{
  switch (timer->type) {
    case POMODORO_TYPE:
      if (timer->pomodoro_count <= 0)
        break;
      timer->type = SHORT_BREAK_TYPE;
      timer->pomodoro_count--;
    break;
    case SHORT_BREAK_TYPE:
      timer->type = POMODORO_TYPE;
    break;
    case LONG_BREAK_TYPE:
      return Timer_initialize(timer);
  }
  if (timer->pomodoro_count <= 0) {
    timer->type = LONG_BREAK_TYPE;
  }
}

void Timer_initialize(Timer *restrict timer)
{
  timer->paused = false;
  timer->pomodoro_count = pomodoro_count;
  timer->type = POMODORO_TYPE;
}

void read_format_from_optind(int argc, char *argv[], const char ** output_str)
{
  while (optind < argc) {
    if (argv[optind][0] == '+') {
      if (*output_str != NULL) {
        errno = 1;
        perror("You've specified multiple format arguments");
        exit(EXIT_FAILURE);
      }
      *output_str = argv[optind++] + 1;
    }
  }
  if (*output_str == NULL)
    *output_str = timer_format;
}

int read_format_from_string(char*input_str,const char ** output_str)
{
  if (input_str[0] == '+') {
    *output_str = input_str+1;
  }
  if (*output_str == NULL) {
    *output_str = timer_format;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

static void divide_seconds_minutes_hours(unsigned int * seconds, unsigned int * minutes, unsigned int * hours)
{
  *hours = (*seconds)/(SECONDS_IN_HOUR);
  *seconds-= (*hours)*SECONDS_IN_HOUR;
  
  *minutes = (*seconds)/SECONDS_IN_MINUTES;
  *seconds = (*seconds)%SECONDS_IN_MINUTES;
}

// Maybe I'll use this some day
char * Timer_time_left(Timer *restrict timer)
{
  unsigned int seconds = timer->seconds, hours, minutes;
  divide_seconds_minutes_hours(&seconds, &minutes, &hours);

  // MIN:SEC
  //  00:00\0
  size_t size = 6;
  if (hours) {
    size_t size_of_hours = int_length(hours);
    size+=size_of_hours+1;
  }
  char * output;
  if (hours) {
    asprintf(&output, "%02u:%02u:%02u", hours, minutes, seconds);
  } else {
    asprintf(&output, "%02u:%02u", minutes, seconds);
  }

  return output;
}

struct timer_format_handler_args{
  Timer *restrict timer;
  char format_char;
};

static char *
Timer_format_character(void *arguments, char format_char)
{
  struct timer_format_handler_args * args = arguments;
  char *str;
  switch (format_char) {
    case 't':
      return Timer_time_left(args->timer);
    case 'p':
      asprintf(&str, "%d", args->timer->pomodoro_count);
    break;
    case 'f':
      fflush(stdout);
      asprintf(&str, "");
    break;
    case 'b':
      asprintf(&str, "%s", Timer_before_time(args->timer->type));
    break;
    default:
      errno = 1;
      perror("Format character not defined");
      exit(1);
    break;
  }
  return str;
}

char * Timer_resolve_format(Timer *restrict timer, char const *format)
{
  struct timer_format_handler_args args = {.timer = timer };
  return resolve_format(format,Timer_format_character, &args);
}

// char * Timer_resolve_format(Timer *restrict timer, char const *format)
// {
//   char * output = "";
//   int output_index = 0;
//   // variable of format current pointer is fmt_ptr
//   for (char const *fmt_ptr = format; *fmt_ptr; fmt_ptr++) {
//     if (fmt_ptr[0] == '%') {
//       char *string_formated = Timer_format_character(timer,fmt_ptr[1]);
//       output_index += asprintf(&output,"%s%s", output,string_formated);
//       free(string_formated);
//       fmt_ptr++;
//     } else {
//       output[output_index] = fmt_ptr[0];
//       output_index += 1;
//     }
//   }
//   return output;
// }

void Timer_print_format(Timer *restrict timer, const char * format)
{
  char * str = Timer_resolve_format(timer, format);
  puts(str);
  free(str);
}

// This method DOES NOT flush the output afterwards.
// Do the flushing yourself (if you need to)
void Timer_print(Timer *restrict timer)
{
  unsigned int seconds = timer->seconds, hours, minutes;
  divide_seconds_minutes_hours(&seconds, &minutes, &hours);

  if (hours) {
    printf("%02u:%02u:%02u", hours, minutes, seconds);
  } else {
    printf("%02u:%02u", minutes, seconds);
  }
}

const char * Timer_before_time(TimerType type)
{
  switch (type) {
    case POMODORO_TYPE:
      if (pomodoro_before_time != NULL)
        return pomodoro_before_time;
    break;
    case SHORT_BREAK_TYPE:
      if (short_break_before_time != NULL)
         return short_break_before_time;
    break;
    case LONG_BREAK_TYPE:
      if (long_break_before_time != NULL)
        return long_break_before_time;
    break;
  }
  return "";
}
void Timer_print_before_time(Timer timer)
{
  const char * before_time = Timer_before_time(timer.type);
  printf("%s", before_time);
}

