#include <linux/limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../include/timer.h"
#include "../include/signal.h"

#include "../config.h"
#include "../include/utils.h"

#ifndef INITIALIZE_APP
  #define APP_NOTIFICATION 0
  #define APP_PRINT_POMODORO_COUNT 0
  #define APP_FLUSH 0
  #define APP_NEW_LINE_AT_QUIT 0
#endif

typedef struct {
  _Bool flush, notification;
  _Bool new_line_at_quit, print_pomodoro_count;
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
  if (app.new_line_at_quit)
    puts("");

  if (signum == SIGQUIT)
    exit(0);
  exit(1);
}

void read_options_to_app(int argc, char*argv[])
{
  int ch;
  while ((ch = getopt(argc, argv, "fnpN")) != -1) {
    switch (ch) {
      case 'f':
        app.flush = !APP_FLUSH;
        break;
      case 'n':
        app.notification = !APP_NOTIFICATION;
        break;
      case 'p':
        app.print_pomodoro_count = !APP_PRINT_POMODORO_COUNT;
        break;
      case 'N':
        app.new_line_at_quit = !APP_NEW_LINE_AT_QUIT;
        break;
    }
  }
}


void run_before_command_based_on_timertype(TimerType type)
{
  switch (type) {
    case POMODORO_TYPE:
      for (unsigned int i = 0; i < LENGTH(ON_POMODORO_START_COMMANDS); i++)
        (void)system(ON_POMODORO_START_COMMANDS[i]);
      break;

    case SHORT_BREAK_TYPE: 
      for (unsigned int i = 0; i < LENGTH(ON_SHORT_BREAK_START_COMMANDS); i++)
        (void)system(ON_SHORT_BREAK_START_COMMANDS[i]);
      break;

    case LONG_BREAK_TYPE: 
      for (unsigned int i = 0; i < LENGTH(ON_LONG_BREAK_START_COMMANDS); i++)
        (void)system(ON_LONG_BREAK_START_COMMANDS[i]);
      break;
  }
}

void print_before_time_based_on_timertype()
{
  switch (timer.type) {
    case POMODORO_TYPE:
      if (POMODORO_BEFORE_TIME_STRING != NULL)
        printf("%s", POMODORO_BEFORE_TIME_STRING);
    break;
    case SHORT_BREAK_TYPE:
      if (SHORT_BREAK_BEFORE_TIME_STRING != NULL)
        printf("%s", SHORT_BREAK_BEFORE_TIME_STRING);
    break;
    case LONG_BREAK_TYPE:
      if (LONG_BREAK_BEFORE_TIME_STRING != NULL)
        printf("%s", LONG_BREAK_BEFORE_TIME_STRING);
    break;
  }
}
void print_all()
{
  print_before_time_based_on_timertype();
  Timer_print_time_left(&timer);

  if (app.print_pomodoro_count) 
    printf("%s%d",BEFORE_POMODORO_COUNT_STRING ,timer.pomodoro_count);
  puts("");
  if (app.flush) 
    fflush(stdout);
}

void send_notification_based_on_timertype(TimerType type)
{
  switch (type) {
    case POMODORO_TYPE:
      send_notification(POMODORO_NOTIF_TITLE, POMODORO_NOTIF_BODY);
      break;

    case SHORT_BREAK_TYPE: 
      send_notification(SHORT_BREAK_NOTIF_TITLE, SHORT_BREAK_NOTIF_BODY);
      break;

    case LONG_BREAK_TYPE: 
      send_notification(LONG_BREAK_NOTIF_TITLE, LONG_BREAK_NOTIF_BODY);
      break;
  }
}


void start_app_loop()
{
  while (1) {
    if (!timer.seconds) {
      if (!timer.paused)
        run_before_command_based_on_timertype(timer.type);

      if (app.notification)
        send_notification_based_on_timertype(timer.type);
    }
    if (!timer.paused) {
      print_all();
    }
    Timer_reduce_second_sleep(&timer);
  }
}

void pause_timer_run_cmds() 
{
  Timer_pause(&timer);
  for (unsigned int i = 0; i < LENGTH(ON_PAUSE_COMMANDS); i++)
    (void)system(ON_PAUSE_COMMANDS[i]);
}

void unpause_timer_run_cmds()
{
  Timer_unpause(&timer);
  for (unsigned int i = 0; i < LENGTH(ON_UNPAUSE_COMMANDS); i++)
    (void)system(ON_UNPAUSE_COMMANDS[i]);
}

void skip_signal_handler(int signum)
{
  Timer_cycle_type(&timer);
  run_before_command_based_on_timertype(timer.type);

  Timer_set_seconds_based_on_type(&timer);

  if (app.notification)
    send_notification_based_on_timertype(timer.type);

  if (timer.paused) {
    print_all();
  }
}

void initialize_app()
{
  #ifdef INITIALIZE_APP
    app.flush = APP_FLUSH;
    app.notification = APP_NOTIFICATION;
    app.print_pomodoro_count = APP_PRINT_POMODORO_COUNT;
    app.new_line_at_quit = APP_NEW_LINE_AT_QUIT;
  #endif
}

void reset_signal_handler()
{
  initialize_app();
  Timer_initialize(&timer);

  Timer_set_seconds_based_on_type(&timer);
  run_before_command_based_on_timertype(timer.type);

  if (timer.paused) {
    print_all();
  }
}


void pause_signal_handler()
{
  pause_timer_run_cmds();
  if (!app.notification)
    return;
  send_notification(PAUSED_NOTIF_TITLE, PAUSED_NOTIF_BODY);
}

void unpause_signal_handler()
{
  unpause_timer_run_cmds();
  if (!app.notification)
    return;
  send_notification(UNPAUSED_NOTIF_TITLE, UNPAUSED_NOTIF_BODY);
}

void toggle_pause_signal_handler()
{
  if (timer.paused)
    pause_timer_run_cmds();
  else
    unpause_timer_run_cmds();

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

  if (timer.paused) 
    print_all();

}

void decrease_10sec_signal_handler()
{
  if (timer.seconds > 10)
    timer.seconds-=10;
  else 
    timer.seconds = 0;

  if (timer.paused)
    print_all();
}

void increase_pomodoro_count_signal_handler()
{
  timer.pomodoro_count ++;

  if (timer.paused)
    print_all();
}

void decrease_pomodoro_count_signal_handler()
{
  if (timer.pomodoro_count > 0)
    timer.pomodoro_count --;

  if (timer.paused)
    print_all();
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
  signal(SIG_INC_POMODORO_COUNT, increase_pomodoro_count_signal_handler);
  signal(SIG_DEC_POMODORO_COUNT, decrease_pomodoro_count_signal_handler);
  signal(SIG_RESET, reset_signal_handler);
}

int main(int argc, char *argv[])
{
  initialize_app();
  Timer_initialize(&timer);

  read_options_to_app(argc, argv);
  Timer_set_seconds_based_on_type(&timer);
  run_before_command_based_on_timertype(timer.type);

  create_pid_file();
  assign_signals_to_handlers();

  if (app.notification)
    send_notification_based_on_timertype(timer.type);

  start_app_loop();

  return EXIT_SUCCESS;
}
