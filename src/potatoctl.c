#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>

#include "../include/signal.h"
#include "../include/client.h"
#include "../include/timer.h"

#define case_index(ch,function) case ch:\
{\
  int index = EVERY_MEMBER;\
  if (optarg != NULL)\
    sscanf(optarg, "%d", &index);\
  run_function_on_pid_file_index(function, index);\
  break;\
}

void handle_list_pid_files(char * pid_str, int index)
{
  int pid = atoi(pid_str);
  Timer timer;
  timer.seconds = send_socket_request_return_num(REQ_SECONDS,pid);
  if (timer.seconds == -1) {
    printf("%d\t%s\n", index, pid_str);
    return;
  }

  timer.type = send_socket_request_return_num(REQ_TYPE,pid);
  timer.pomodoro_count = send_socket_request_return_num(REQ_POMODOROS,pid);
  timer.paused = send_socket_request_return_num(REQ_PAUSED, pid);

  char * is_paused = timer.paused ? "Yes" : "No";

  char * time_left = Timer_time_left(&timer);
  printf("%d\t%s\t%s\t%s\t%d\n", index, pid_str, time_left, is_paused, timer.pomodoro_count);
  free(time_left);
}

void get_type(char *pid_str, int index)
{
  printf("%d\n",send_socket_request_return_num(REQ_TYPE,atoi(pid_str)));
}

void get_seconds(char *pid_str, int index)
{
  printf("%d\n", send_socket_request_return_num(REQ_SECONDS,atoi(pid_str)));
}

void remove_potato_pid_file(char *name, int index)
{
  char path[PATH_MAX];
  snprintf(path, PATH_MAX, "%s/%s", POTATO_PIDS_DIRECTORY, name);

  remove(path);
}

int main(int argc, char *argv[])
{   
  int ch;
  while ((ch = getopt(argc, argv, "T::S::c::lu::L::s::p::t::q::d::i::I::D::r::")) != -1) {
    switch (ch) {
      case 'l': 
        puts("INDEX\tPID\tTIME\tPAUSED\tPOMODOROS");
        run_function_on_pid_file_index(handle_list_pid_files, EVERY_MEMBER);
      break;

      case_index('c', remove_potato_pid_file);
      case_index('p', handle_pause);
      case_index('u', handle_unpause);
      case_index('t', handle_toggle_pause);
      case_index('q', handle_quit);
      case_index('s', handle_skip);
      case_index('i', handle_increase_10sec);
      case_index('d', handle_decrease_10sec);
      case_index('I', handle_increase_pomodoro_count);
      case_index('D', handle_decrease_pomodoro_count);
      case_index('r', handle_reset_pomodoro);
      case_index('T', get_type);
      case_index('S', get_seconds);
    }
  }
  return EXIT_SUCCESS;
}
