#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include "../config.h"
#include "../include/timer.h"

void Timer_pause(Timer * timer)
{
  timer->paused = 1;
}

void Timer_unpause(Timer * timer)
{
  timer->paused = 0;
}

void Timer_toggle_pause(Timer * timer)
{
  if (timer->paused)
    Timer_unpause(timer);
  else
    Timer_pause(timer);
}

// This method has been designed in a way to just be 
// put inside a loop and just work for you as your timer.
// You can use `Timer_print_time_left` function afterwards
void Timer_reduce_second_sleep(Timer * timer)
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

void Timer_set_seconds_based_on_type(Timer * timer)
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

void Timer_cycle_type(Timer * timer)
{
  switch (timer->type) {
    case POMODORO_TYPE:
      timer->type = SHORT_BREAK_TYPE;
      timer->pomodoro_count--;
    break;
    case SHORT_BREAK_TYPE:
      timer->type = POMODORO_TYPE;
    break;
    case LONG_BREAK_TYPE:
      return Timer_initialize(timer);
  }
  if (timer->pomodoro_count == 0)
    timer->type = LONG_BREAK_TYPE;
}

void Timer_initialize(Timer *timer)
{
  timer->paused = 0;
  timer->pomodoro_count = POMODORO_COUNT;
  timer->type = POMODORO_TYPE;
}

// This method DOES NOT flush the output afterwards.
// Do the flushing yourself
void Timer_print_time_left(Timer *timer)
{
  int seconds = timer->seconds;
  int hours = seconds/(SECONDS_IN_HOUR);
  seconds-= hours*SECONDS_IN_HOUR;
  
  int minutes = seconds/SECONDS_IN_MINUTES;
  seconds = seconds%SECONDS_IN_MINUTES;

  if (hours) {
    printf("%02d:%02d:%02d", hours, minutes, seconds);
  } else {
    printf("%02d:%02d", minutes, seconds);
  }
}

