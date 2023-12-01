#include <linux/limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/timer.h"
#include "../include/signal.h"
#include "../include/socket.h"
#include "../include/pidfile.h"

#include "../config.h"
#include "../include/utils.h"

#ifndef INITIALIZE_APP
  #define APP_NOTIFICATION 0
  #define APP_PRINT_POMODORO_COUNT 0
  #define APP_FLUSH 0
  #define APP_NEW_LINE_AT_QUIT 0
  #define APP_RUN_SOCKET 1
#endif

typedef struct {
  _Bool flush, notification;
  _Bool new_line_at_quit, print_pomodoro_count, run_socket;
} App;

Timer timer;
App app;

void quit(int signum)
{
  remove_pid_file(getpid());
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

void print_all()
{
  Timer_print_before_time(timer);
  Timer_print(&timer);

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
    Timer_sleep_reduce_second(&timer);
  }
}

void pause_timer_run_cmds() 
{
  if (timer.paused)
    return;
  Timer_pause(&timer);
  for (unsigned int i = 0; i < LENGTH(ON_PAUSE_COMMANDS); i++)
    (void)system(ON_PAUSE_COMMANDS[i]);
}

void unpause_timer_run_cmds()
{
  if (!timer.paused)
    return;
  Timer_unpause(&timer);
  for (unsigned int i = 0; i < LENGTH(ON_UNPAUSE_COMMANDS); i++)
    (void)system(ON_UNPAUSE_COMMANDS[i]);
}

void skip_signal_handler(int signum)
{
  Timer_cycle_type(&timer);
  Timer_set_seconds_based_on_type(&timer);

  run_before_command_based_on_timertype(timer.type);

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
    app.run_socket = APP_RUN_SOCKET;
  #endif
}

void reset_signal_handler()
{
  initialize_app();

  Timer_initialize(&timer);
  Timer_set_seconds_based_on_type(&timer);

  run_before_command_based_on_timertype(timer.type);

  print_all();
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
    unpause_timer_run_cmds();
  else
    pause_timer_run_cmds();

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
  signal(SIGINT, quit);
  signal(SIGILL, quit);
  signal(SIGABRT, quit);
  signal(SIGFPE, quit);
  signal(SIGSEGV, quit);
  signal(SIGTERM, quit);
  signal(SIGQUIT, quit);
  signal(SIGTRAP, quit);
  signal(SIGKILL, quit);
  signal(SIGPIPE, quit);
  signal(SIGALRM, quit);


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


void *run_sock_server_thread(void *arg)
{
  int port = next_available_sock_port();
  write_sock_port_to_pid_file(getpid(), port);
  if (port == NO_PORT) {
    puts("Unable to create socket.");
    fflush(stdout);
    pthread_exit(NULL);
  }
  int server_fd, new_socket;
  ssize_t valread;
  struct sockaddr_in address;
  int opt = 1;
  socklen_t addrlen = sizeof(address);
  char buffer[1024] = { 0 };

  // Creating socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket failed");
      exit(EXIT_FAILURE);
  }

  // Forcefully attaching socket to the port 8080
  if (setsockopt(server_fd, SOL_SOCKET,
                 SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
      perror("setsockopt");
      exit(EXIT_FAILURE);
  }
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  // Forcefully attaching socket to the port 8080
  if (bind(server_fd, (struct sockaddr*)&address,
           sizeof(address))
      < 0) {
      perror("bind failed");
      exit(EXIT_FAILURE);
  }
  if (listen(server_fd, 3) < 0) {
      perror("listen");
      exit(EXIT_FAILURE);
  }
  while (1) {
    if ((new_socket
         = accept(server_fd, (struct sockaddr*)&address,
                  &addrlen))
        < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    valread = read(new_socket, buffer, 1024 - 1);

    if (!valread)
      continue;

    SocketRequest req;
    sscanf(buffer, "%d", &req);

    char * message;
    size_t message_len;
    int number;
    switch (req) {
      case REQ_SECONDS: {
        number = timer.seconds;
        break;
      }
      case REQ_TYPE: {
        number = timer.type;
        break;
      }
      case REQ_POMODOROS: {
        number = timer.pomodoro_count;
        break;
      }
      case REQ_PAUSED: {
        number = timer.paused;
        break;
      }
      case REQ_TIMER_FULL: {
        message_len = int_length(timer.type)+int_length(timer.seconds)+int_length(timer.pomodoro_count)+int_length(timer.paused)+4;
        message = malloc(message_len*sizeof(char));
        snprintf(message, message_len, "%d-%d-%d-%d", timer.seconds, timer.pomodoro_count, timer.paused, timer.type);
        send(new_socket, message, message_len, 0);
        close(new_socket);
      }
    }
    if (req != REQ_TIMER_FULL) {
      message_len = int_length(number)+1;
      message = malloc(message_len*sizeof(char));
      snprintf(message, message_len, "%d", number);

      send(new_socket, message, message_len, 0);
      close(new_socket);
    }
    free(message);
  }
  pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{

  initialize_app();
  read_options_to_app(argc, argv);

  Timer_initialize(&timer);
  Timer_set_seconds_based_on_type(&timer);

  run_before_command_based_on_timertype(timer.type);

  create_pid_file(getpid());
  assign_signals_to_handlers();

  if (app.run_socket) {
    pthread_t thread1;
    pthread_create(&thread1, NULL, run_sock_server_thread, NULL);
  }

  if (app.notification)
    send_notification_based_on_timertype(timer.type);

  start_app_loop();

  return EXIT_SUCCESS;
}
