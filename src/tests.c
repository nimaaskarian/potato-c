#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../include/timer.h"
#include "../include/utils.h"
#include "../config.h"

void test_timer_init(Timer *timer)
{
  Timer_initialize(timer);
  assert(timer->paused == 0);
  assert(timer->seconds == 0);
  Timer_set_seconds_based_on_type(timer);
  assert(timer->seconds == POMODORO_MINUTES*SECONDS_IN_MINUTES);
  assert(timer->type == POMODORO_TYPE);
}

void test_timer_procedure(Timer *timer, int depth)
{
  if (depth)
    return;

  Timer_toggle_pause(timer);
  assert(timer->paused);
  Timer_toggle_pause(timer);
  assert(!timer->paused);
  Timer_pause(timer);
  assert(timer->paused);
  Timer_unpause(timer);
  assert(!timer->paused);

  #define I_START 3
  int i = I_START;
  while (i--) {
    Timer_reduce_second_sleep(timer);
  }
  assert(timer->seconds == POMODORO_MINUTES*SECONDS_IN_MINUTES-I_START);
  Timer_cycle_type(timer);
  if (timer->pomodoro_count)
    assert(timer->type == SHORT_BREAK_TYPE);
  else {
    assert(timer->type == LONG_BREAK_TYPE);
    Timer_initialize(timer);
    Timer_set_seconds_based_on_type(timer);
    timer->pomodoro_count+=1;
    test_timer_procedure(timer, depth+1);
  }
}

int main(int argc, char *argv[])
{
  assert(int_length(0) == 1);
  assert(int_length(100) == 3);

  Timer timer;
  test_timer_init(&timer);
  test_timer_procedure(&timer, 0);
  return EXIT_SUCCESS;
}
