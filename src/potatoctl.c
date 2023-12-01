#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>

#include "../config.h"
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
  Timer timer = get_timer_pid(pid);
  if (timer.type == NULL_TYPE) {
    printf("%d\t%s\n", index, pid_str);
    return;
  }

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
void get_timer_one_time(char *pid_str, int index)
{
  Timer timer;
  timer = get_timer_pid(atoi(pid_str));
  if (timer.type == NULL_TYPE)
    return;

  Timer_print_before_time(timer);
  Timer_print(&timer);
  printf("%s%d",BEFORE_POMODORO_COUNT_STRING ,timer.pomodoro_count);
  puts("");
  fflush(stdout);
}
void get_timer_each_second(char *pid_str, int index)
{
  Timer timer;
  while (1) {
    timer = get_timer_pid(atoi(pid_str));
    if (timer.type == NULL_TYPE)
      break;

    Timer_print_before_time(timer);
    Timer_print(&timer);
    printf("%s%d",BEFORE_POMODORO_COUNT_STRING ,timer.pomodoro_count);
    puts("");
    fflush(stdout);
    sleep(1);
  }
}

void list_all_timers()
{
  if (get_pids_length()) {
    puts("INDEX\tPID\tTIME\tPAUSED\tPOMODOROS");
    run_function_on_pid_file_index(handle_list_pid_files, EVERY_MEMBER);
  }
}

int main(int argc, char *argv[])
{   
  if (argc < 2) {
    list_all_timers();
    return EXIT_SUCCESS;
  }
  int ch;
  while ((ch = getopt(argc, argv, "1::T::S::c::lu::L::s::p::t::q::d::i::I::D::r::")) != -1) {
    switch (ch) {
      case 'l': 
        list_all_timers();
      break;

      case_index('c', handle_remove_pid);
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
      case_index('T', get_timer_each_second);
      case_index('1', get_timer_one_time);
    }
  }
  return EXIT_SUCCESS;
}
