#include <math.h>
#include <stdio.h>
#include <time.h>
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
void Timer_reduce_second_sleep(Timer *restrict timer)
{
  if (!timer->paused) {
    timer->seconds--;
  }
  if (!timer->seconds) {
    Timer_cycle_type(timer);
    Timer_set_seconds_based_on_type(timer);
  }
  sleep(1);
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

