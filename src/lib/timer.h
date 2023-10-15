#ifndef TIMER_H_
#define TIMER_H_
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define SECONDS_IN_MINUTES 60


typedef enum { 
  POMODORO_TYPE = 0,
  SHORT_PAUSE_TYPE,
  LONG_PAUSE_TYPE,
} TimerType;

typedef struct {
  _Bool paused;
  uint seconds;
  TimerType type;
  uint pomodoro_count;
} Timer;

void initialize_timer(Timer *timer);
void reduce_timer_second_not_paused(Timer * timer);
float minutes_of_timer_type(TimerType type);
void set_timer_seconds_based_on_type(Timer * timer);
void cycle_type(Timer * timer);
void initialize_timer(Timer *timer);
void print_time_left(Timer *timer);
void pause_timer(Timer * timer);
void unpause_timer(Timer * timer);
void toggle_pause_timer(Timer * timer);
void send_notification_based_on_timertype(TimerType type);
#endif

