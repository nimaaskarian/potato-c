#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "../config.h"
#include "../include/timer.h"
#include "../include/utils.h"

extern inline void Timer_pause(Timer *restrict timer)
{
  timer->paused = true;
}

extern inline void Timer_unpause(Timer *restrict timer)
{
  timer->paused = false;
}

extern inline void Timer_toggle_pause(Timer *restrict timer)
{
  if (timer->paused)
    Timer_unpause(timer);
  else
    Timer_pause(timer);
}

// This method has been designed in a way to just be 
// put inside a loop and just work for you as your timer.
// You can use `Timer_print_time_left` function afterwards
extern inline void Timer_sleep_reduce_second(Timer *restrict timer, void on_cycle(Timer * restrict))
{
  sleep(1);
  if (!timer->paused) {
    timer->seconds--;
  }
  if (!timer->seconds) {
    Timer_cycle_type(timer);
    Timer_set_seconds_based_on_type(timer);
    on_cycle(timer);
  }
}

extern inline void Timer_set_seconds_based_on_type(Timer *restrict timer)
{
  float minutes;
  switch (timer->type) {
    case POMODORO_TYPE:
      minutes = timer->pomodoro_minutes;
    break;
    case SHORT_BREAK_TYPE:
      minutes = timer->short_break_minutes;
    break;
    case LONG_BREAK_TYPE:
      minutes = timer->long_break_minutes;
    break;

    case NULL_TYPE:
      break;
  }
  timer->seconds = minutes*SECONDS_IN_MINUTES;
}

extern inline void Timer_cycle_type(Timer *restrict timer)
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
    case NULL_TYPE:
      break;
  }
  if (timer->pomodoro_count <= 0) {
    timer->type = LONG_BREAK_TYPE;
  }
}

extern inline void Timer_initialize(Timer *restrict timer)
{
  timer->pomodoro_count = pomodoro_count;
  timer->type = POMODORO_TYPE;
}

extern inline void Timer_set_default_time(Timer *restrict timer)
{
  timer->long_break_minutes = default_long_break_minutes;
  timer->short_break_minutes = default_short_break_minutes;
  timer->pomodoro_minutes = default_pomodoro_minutes;
}

extern inline void read_format_from_optind(int argc, char *argv[], const char ** output_str)
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

extern inline int read_format_from_string(char*restrict input_str,const char **restrict output_str)
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

inline static void divide_seconds_minutes_hours(unsigned int * seconds, unsigned int * minutes, unsigned int * hours)
{
  *hours = (*seconds)/(SECONDS_IN_HOUR);
  *seconds-= (*hours)*SECONDS_IN_HOUR;
  
  *minutes = (*seconds)/SECONDS_IN_MINUTES;
  *seconds = (*seconds)%SECONDS_IN_MINUTES;
}

// Maybe I'll use this some day
extern inline char * Timer_time_left(Timer *restrict timer)
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

inline extern const char * timer_type_string(TimerType type)
{
  switch (type) {
    case NULL_TYPE:
      return "Invalid";
    case POMODORO_TYPE:
      return "Pomodoro";
    case SHORT_BREAK_TYPE:
      return "Short break";
    case LONG_BREAK_TYPE:
      return "Long break";
  }
}

inline static char *
Timer_format_character(void *restrict arguments, char format_char)
{
  struct timer_format_handler_args * args = arguments;
  char *str;
  switch (format_char) {
    case 't':
      return Timer_time_left(args->timer);
    case 'p':
      asprintf(&str, "%d", args->timer->pomodoro_count);
    break;
    case 'm':
      asprintf(&str, "%s", timer_type_string(args->timer->type));
    break;
    case 'f':
      fflush(stdout);
      str = malloc(0);
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

extern inline char * Timer_resolve_format(Timer *restrict timer, char const *format)
{
  struct timer_format_handler_args args = {.timer = timer };
  return resolve_format(format,Timer_format_character, &args);
}

extern inline void Timer_print_format(Timer *restrict timer, const char * format)
{
  char * str = Timer_resolve_format(timer, format);
  puts(str);
  free(str);
}

// This method DOES NOT flush the output afterwards.
// Do the flushing yourself (if you need to)
extern inline void Timer_print(Timer *restrict timer)
{
  unsigned int seconds = timer->seconds, hours, minutes;
  divide_seconds_minutes_hours(&seconds, &minutes, &hours);

  if (hours) {
    printf("%02u:%02u:%02u", hours, minutes, seconds);
  } else {
    printf("%02u:%02u", minutes, seconds);
  }
}

extern inline const char * Timer_before_time(TimerType type)
{
  switch (type) {
    case POMODORO_TYPE:
      if (pomodoro_before_time != NULL)
        return pomodoro_before_time;
    case SHORT_BREAK_TYPE:
      if (short_break_before_time != NULL)
         return short_break_before_time;
    case LONG_BREAK_TYPE:
      if (long_break_before_time != NULL)
        return long_break_before_time;
    case NULL_TYPE:
      return "";
  }
}
extern inline void Timer_print_before_time(Timer timer)
{
  const char * before_time = Timer_before_time(timer.type);
  printf("%s", before_time);
}

