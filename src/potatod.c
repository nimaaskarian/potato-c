#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lib/timer.h"
#include "lib/signal.h"

#include "config.h"
#include "lib/utils.h"


typedef struct {
  _Bool flush, notification;
} App;

Timer timer;
App app;


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

void read_options_to_app(int argc, char*argv[])
{
  int ch;
  while ((ch = getopt(argc, argv, "fn")) != -1) {
    switch (ch) {
      case 'f':
        app.flush = 1;
        break;
      case 'n':
        app.notification = 1;
        break;
    }
  }
}

void print_before_time_based_on_timertype()
{
  switch (timer.type) {
    case POMODORO_TYPE:
      if (POMODORO_BEFORE_TIME_STRING != NULL)
        printf("%s", POMODORO_BEFORE_TIME_STRING);
    break;
    case SHORT_PAUSE_TYPE:
      if (SHORT_PAUSE_BEFORE_TIME_STRING != NULL)
        printf("%s", SHORT_PAUSE_BEFORE_TIME_STRING);
    break;
    case LONG_PAUSE_TYPE:
      if (LONG_PAUSE_BEFORE_TIME_STRING != NULL)
        printf("%s", LONG_PAUSE_BEFORE_TIME_STRING);
    break;
  }
}

void start_app_loop()
{
    while (1) {
    if (!timer.seconds) {
      if (timer.type == LONG_PAUSE_TYPE)
        initialize_timer(&timer);
      else 
        cycle_type(&timer);
      set_timer_seconds_based_on_type(&timer);

      if (app.notification)
        send_notification_based_on_timertype(timer.type);
    }
    if (!timer.paused) {
      print_before_time_based_on_timertype();
      print_time_left(&timer);
    }
    if (app.flush) 
      fflush(stdout);
    reduce_timer_second_not_paused(&timer);

    sleep(1);
  }
}

void skip_signal_handler(int signum)
{
  if (timer.type == LONG_PAUSE_TYPE)
    initialize_timer(&timer);
  else
    cycle_type(&timer);

  set_timer_seconds_based_on_type(&timer);

  if (app.notification)
    send_notification_based_on_timertype(timer.type);

  if (timer.paused) {
    print_before_time_based_on_timertype();
    print_time_left(&timer);
  }
}

void pause_signal_handler(int signum)
{
  pause_timer(&timer);
  if (!app.notification)
    return;
  send_notification(PAUSED_NOTIF_TITLE, PAUSED_NOTIF_BODY);
}

void unpause_signal_handler(int signum)
{
  unpause_timer(&timer);
  if (!app.notification)
    return;
  send_notification(UNPAUSED_NOTIF_TITLE, UNPAUSED_NOTIF_BODY);
}

void toggle_pause_signal_handler(int signum)
{
  toggle_pause_timer(&timer);
  if (!app.notification)
    return;
  if (timer.paused)
    send_notification(PAUSED_NOTIF_TITLE, PAUSED_NOTIF_BODY);
  else 
    send_notification(UNPAUSED_NOTIF_TITLE, UNPAUSED_NOTIF_BODY);
}

void increase_10sec_signal_handler()
{
  timer.seconds+=10;

  if (timer.paused) {
    print_before_time_based_on_timertype();
    print_time_left(&timer);
  }
}

void decrease_10sec_signal_handler()
{
  if (timer.seconds > 10)
    timer.seconds-=10;
  else 
    timer.seconds = 0;

  if (timer.paused) {
    print_before_time_based_on_timertype();
    print_time_left(&timer);
  }

}

void assign_signals_to_handlers()
{
  signal(SIGQUIT, quit);
  signal(SIGINT, quit);
  signal(SIGTERM, quit);
  signal(SIG_PAUSE, pause_signal_handler);
  signal(SIG_UNPAUSE, unpause_signal_handler);
  signal(SIG_TPAUSE, toggle_pause_signal_handler);
  signal(SIG_SKIP, skip_signal_handler);
  signal(SIG_INC_10SEC, increase_10sec_signal_handler);
  signal(SIG_DEC_10SEC, decrease_10sec_signal_handler);
}

void initialize_app()
{
  app.flush = 0;
  app.notification = 0;
}

int main(int argc, char *argv[])
{
  initialize_app();
  initialize_timer(&timer);

  read_options_to_app(argc, argv);
  set_timer_seconds_based_on_type(&timer);

  create_pid_file();
  assign_signals_to_handlers();

  if (app.notification)
    send_notification_based_on_timertype(timer.type);

  start_app_loop();

  return EXIT_SUCCESS;
}
