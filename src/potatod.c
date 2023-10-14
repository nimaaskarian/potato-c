#include "lib/timer.h"
#include "lib/signal.h"

#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

Timer timer;

typedef struct {
  _Bool flush;
} App;

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

void create_pid_file()
{
  struct stat st = {0};
  if (stat(POTATO_PIDS_DIRECTORY, &st) == -1) {
    mkdir(POTATO_PIDS_DIRECTORY, 0700);
  }

  char pid_path[PATH_MAX];
  snprintf(pid_path, PATH_MAX, "%s/%d", POTATO_PIDS_DIRECTORY, getpid());

  FILE* file_ptr = fopen(pid_path, "w");
  fclose(file_ptr);
}

void remove_pid_file()
{
  char pid_path[PATH_MAX];
  snprintf(pid_path, PATH_MAX, "%s/%d", POTATO_PIDS_DIRECTORY, getpid());

  remove(pid_path);
}

void quit(int signum)
{
  remove_pid_file();
  if (signum == SIGQUIT)
    exit(0);
  exit(1);

}

void read_options(int argc, char*argv[], App * app)
{
  int ch;
  while ((ch = getopt(argc, argv, "f")) != -1) {
    switch (ch) {
      case 'f':
        app->flush = 1;
        break;
    }
  }
}

void start_app_loop(Timer* timer, App app)
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
    if (app.flush) 
      fflush(stdout);
    reduce_timer_second_not_paused(timer);

    sleep(1);
  }
}

void assign_signals_to_functions()
{
  signal(SIGQUIT, quit);
  signal(SIGINT, quit);
  signal(SIGTERM, quit);

  signal(SIG_PAUSE, pause_signal_handler);
  signal(SIG_UNPAUSE, unpause_signal_handler);
  signal(SIG_TPAUSE, toggle_pause_signal_handler);
  signal(SIG_SKIP, skip_signal_handler);


}
int main(int argc, char *argv[])
{ 
  App app;
  app.flush = 0;

  create_pid_file();
  assign_signals_to_functions();

  initialize_timer(&timer);
  set_timer_seconds(&timer);

  send_notification_based_on_timertype(timer.type);

  read_options(argc, argv, &app);
  start_app_loop(&timer, app);

  return EXIT_SUCCESS;
}
