#include "lib/timer.h"
#include "lib/signal.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

Timer timer;

void skip_signal_handler(int signum)
{
  cycle_type(&timer);
  set_timer_seconds(&timer);
  send_notification_based_on_timertype(timer.type);
}
void pause_signal_handler(int signum)
{
  pause_timer(&timer);
}
void unpause_signal_handler(int signum)
{
  unpause_timer(&timer);
}
void toggle_pause_signal_handler(int signum)
{
  toggle_pause_timer(&timer);
}

void loop_timer(Timer *timer)
{
  while (1) {
    if (!timer->seconds) {
      if (timer->type == LONG_PAUSE_TYPE)
        initialize_timer(timer);
      else 
        cycle_type(timer);
      send_notification_based_on_timertype(timer->type);

      set_timer_seconds(timer);
    }
    print_time_left_not_paused(timer);
    reduce_timer_second_not_paused(timer);

    sleep(1);
  }
}

void create_pid_file()
{
  struct stat st = {0};
  if (stat(POTATO_PIDS_DIRECTORY, &st) == -1) {
    mkdir(POTATO_PIDS_DIRECTORY, 0700);
  }

  char pid_path[256];
  snprintf(pid_path, 256, "%s/%d", POTATO_PIDS_DIRECTORY, getpid());

  FILE* file_ptr = fopen(pid_path, "w");
  fclose(file_ptr);
}

void remove_pid_file()
{
  char pid_path[256];
  snprintf(pid_path, 256, "%s/%d", POTATO_PIDS_DIRECTORY, getpid());

  remove(pid_path);
}

void quit(int signum)
{
  remove_pid_file();
  if (signum == SIGQUIT)
    exit(0);
  exit(1);

}

int main(void)
{ 
  create_pid_file();

  signal(SIGQUIT, quit);
  signal(SIGINT, quit);
  signal(SIGTERM, quit);

  signal(SIG_PAUSE, pause_signal_handler);
  signal(SIG_UNPAUSE, unpause_signal_handler);
  signal(SIG_TPAUSE, toggle_pause_signal_handler);
  signal(SIG_SKIP, skip_signal_handler);

  initialize_timer(&timer);
  send_notification_based_on_timertype(timer.type);

  set_timer_seconds(&timer);

  loop_timer(&timer);

  return EXIT_SUCCESS;
}
