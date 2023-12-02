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
  timer->paused = 1;
}

void Timer_unpause(Timer *restrict timer)
{
  timer->paused = 0;
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
      minutes = POMODORO_MINUTES;
    break;
    case SHORT_BREAK_TYPE:
      minutes = SHORT_BREAK_MINUTES;
    break;
    case LONG_BREAK_TYPE:
      minutes = LONG_BREAK_MINUTES;
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
  timer->paused = 0;
  timer->pomodoro_count = POMODORO_COUNT;
  timer->type = POMODORO_TYPE;
}

void read_format_from_optind(int argc, char *argv[], char ** output_str)
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
    *output_str = DEFAULT_FORMAT;
}

int read_format_from_string(char*input_str,char ** output_str)
{
  if (input_str[0] == '+') {
    *output_str = input_str+1;
  }
  if (*output_str == NULL) {
    *output_str = DEFAULT_FORMAT;
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
  char * output = malloc(sizeof(char) * size);
  if (hours) {
    snprintf(output, size, "%02u:%02u:%02u", hours, minutes, seconds);
  } else {
    snprintf(output, size, "%02u:%02u", minutes, seconds);
  }

  return output;
}

static char *
Timer_format_character(Timer *restrict timer, char format_char)
{
  char *str = malloc(sizeof(char)*101);
  switch (format_char) {
    case 't':
      free(str);
      return Timer_time_left(timer);
    case 'p':
      snprintf(str,100, "%d", timer->pomodoro_count);
    break;
    case 'f':
      fflush(stdout);
      strcpy(str, "");
    break;
    case 'b':
      snprintf(str,100, "%s", Timer_before_time(timer->type));
    break;
    case 'B':
      snprintf(str,100, "%s", BEFORE_POMODORO_COUNT_STRING);
    break;
    case '%':
      strcpy(str, "%");
    break;
    default:
      errno = 1;
      perror("Format character not defined");
      exit(1);
    break;
  }
  return str;
}

char * Timer_resolve_format(Timer *restrict timer, char const *format, char output[4096])
{
  int output_index = 0;
  // variable of format current pointer is fmt_ptr
  for (char const *fmt_ptr = format; *fmt_ptr; fmt_ptr++) {
    if (fmt_ptr[0] == '%') {
      char *string_formated = Timer_format_character(timer,fmt_ptr[1]);
      output_index += snprintf(&output[output_index],4096-output_index, "%s", string_formated);
      free(string_formated);
      fmt_ptr++;
    } else {
      output[output_index] = fmt_ptr[0];
      output_index += 1;
    }
  }
  return output;
}

void Timer_print_format(Timer *restrict timer, const char * format)
{
  char str[4096];
  Timer_resolve_format(timer, format, str);
  puts(str);
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
      if (POMODORO_BEFORE_TIME_STRING != NULL)
        return POMODORO_BEFORE_TIME_STRING;
    break;
    case SHORT_BREAK_TYPE:
      if (SHORT_BREAK_BEFORE_TIME_STRING != NULL)
         return SHORT_BREAK_BEFORE_TIME_STRING;
    break;
    case LONG_BREAK_TYPE:
      if (LONG_BREAK_BEFORE_TIME_STRING != NULL)
        return LONG_BREAK_BEFORE_TIME_STRING;
    break;
    case NULL_TYPE:
      return "";
    return "";
  }
}
void Timer_print_before_time(Timer timer)
{
  const char * before_time = Timer_before_time(timer.type);
  printf("%s", before_time);
}

