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
  timer->pomodoro_count = timer->initial_pomodoro_count;
  timer->type = POMODORO_TYPE;
}

extern inline void Timer_set_default(Timer *restrict timer)
{
  *timer = default_timer;
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

typedef struct {
  unsigned int seconds, minutes, hours;
} NaiveTime;
inline static NaiveTime divide_seconds_to_naive_time(unsigned int seconds)
{
  NaiveTime output;
  
  output.hours = (seconds)/(SECONDS_IN_HOUR);
  seconds -= (output.hours)*SECONDS_IN_HOUR;
  
  output.minutes = (seconds)/SECONDS_IN_MINUTES;
  output.seconds = (seconds)%SECONDS_IN_MINUTES;

  return output;
}

char * NaiveTime_to_string(NaiveTime time) {
  char * output;
  if (time.hours) {
    asprintf(&output, "%02u:%02u:%02u", time.hours, time.minutes, time.seconds);
  } else {
    asprintf(&output, "%02u:%02u", time.minutes, time.seconds);
  }
  return output;
}

extern inline char * Timer_time_left(Timer *restrict timer)
{
  NaiveTime time_left = divide_seconds_to_naive_time(timer->seconds);

  return NaiveTime_to_string(time_left);
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
      asprintf(&str, "%u", args->timer->pomodoro_count);
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
  char * timer_str = Timer_time_left(timer);
  fputs(timer_str, stdout);
  free(timer_str);
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

