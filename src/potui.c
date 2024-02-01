#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <ncurses.h>

#include "../include/timer.h"
#include "../include/client.h"
#include "../include/utils.h"
#include "../include/ncurses-utils.h"

enum QUIT_MENU {
  QUIT_NONE = 0,
  QUIT_ESC,
  QUIT_QUIT,
};
enum QUIT_MENU input_quit_menu(int ch)
{
  switch (ch) {
    case 'q':
    case 'Q': 
      return QUIT_QUIT;
    break;
    case ESC:
      return QUIT_ESC;
    break;
  }
  return QUIT_NONE;
}

void printw_pid(char *pid_str, int index)
{
  mvprintw(index, 0, "%s\n", pid_str);
}

typedef struct {
  int length;
  int index;
} DrawPidsArgs;

void check_pids_length_and_set_selected(DrawPidsArgs * args)
{
  if (args->index < 0) {
    args->index = args->length ? args->length-1 : 0;
    return;
  }

  if (args->index >= args->length)  {
    args->index = 0;
  }
}

pid_t handle_input_pid_menu(int ch, DrawPidsArgs *args)
{
  if (args->length == 1)
    return pid_at_index(0);
  pid_t pid = 0;
  switch (ch) {
    case 'j':
      args->index++;
      check_pids_length_and_set_selected(args);
      break;
    case 'k':
      args->index--;
      check_pids_length_and_set_selected(args);
      break;
    case 'g':
      args->index = 0;
      break;
    case 'c':
      run_function_on_pid_file_index(handle_remove_pid, args->index);
      args->length--;
      break;
    case 'D':
      run_function_on_pid_file_index(handle_quit, args->index);
      args->length--;
      break;
    case 'G':
      args->index = args->length - 1;
      break;
    case '\n':
      pid = pid_at_index(args->index);
    break;
  }
  if (args->index >= args->length)
    args->index = args->length ? args->length - 1 : 0;
  return pid;
}

void draw_pids(DrawPidsArgs * args)
{
  erase();
  if (args->length) {
    run_function_on_pid_file_index(printw_pid,EVERY_MEMBER);
    attron(COLOR_PAIR(1));
    run_function_on_pid_file_index(printw_pid,args->index);
    attroff(COLOR_PAIR(1));
  } else {
    mvprintw(0,0, "No daemons found right now.");
  }
  refresh();
}

void * get_pids_length_sleep(void* arguments)
{
  DrawPidsArgs * args = arguments;
  while (1) {
    args->length = get_pids_length();
    draw_pids(args);
    sleep(2);
  }
  pthread_exit(EXIT_SUCCESS);
}

pid_t pid_selection_menu()
{
  DrawPidsArgs args = {.length = 0, .index = 0};
  pthread_t get_pids_thread;
  pthread_create(&get_pids_thread, NULL, get_pids_length_sleep, &args);

  pid_t selected_pid;
  draw_pids(&args);

  while (1) {
    int ch = getch();
    if (input_quit_menu(ch) == QUIT_QUIT) ncurses_quit();
    selected_pid = handle_input_pid_menu(ch, &args);
    draw_pids(&args);

    if (selected_pid)
      break;
  }
  pthread_cancel(get_pids_thread);
  return selected_pid;
}

pid_t pid;
_Bool timer_thread_paused = 0;

int handle_input_timer(int ch)
{
  switch (ch) {
    case 'p':
    case 'P':
    case ' ':
      run_function_on_pid_file_pid(handle_toggle_pause, pid);
    break;
    case '_':
    case '-':
      run_function_on_pid_file_pid(handle_decrease_pomodoro_count, pid);
    break;
    case '=':
    case '+':
      run_function_on_pid_file_pid(handle_increase_pomodoro_count, pid);
    break;
    case 'l':
      run_function_on_pid_file_pid(handle_decrease_10sec, pid);
    break;
    case 'h':
      run_function_on_pid_file_pid(handle_increase_10sec, pid);
    break;
    case 's':
      run_function_on_pid_file_pid(handle_skip, pid);
    break;
    case 'r':
      run_function_on_pid_file_pid(handle_reset_pomodoro, pid);
    break;
    // case 'd':
    case 'D':
      run_function_on_pid_file_pid(handle_quit, pid);
    break;
    case 'q':
      ncurses_quit();
    case ESC:
      return 1;
  }
  return 0;
}

typedef struct {
  Timer timer;
  TimerType last_type;
} PrintTimerArgs;

void printw_timer(PrintTimerArgs * args) 
{
  if (!args->timer.paused || args->timer.type != args->last_type) {
    ncurses_clear_lines_from_to(0,3);
    args->last_type = args->timer.type;
  }
  char * time_left_str = Timer_time_left(&args->timer);
  mvprintw(0,0, "Time left: %s", time_left_str);
  mvprintw(1,0, "Pomodoros: %d", args->timer.pomodoro_count);
  const char * type_string = timer_type_string(args->timer.type);
  mvprintw(2,0, "Type: %s", type_string);
  free(time_left_str);
  refresh();
}

void * get_and_printw_timer(void * arguments)
{
  PrintTimerArgs * args = arguments;
  while (1) {
    if (pid)
      args->timer = get_local_timer_from_pid(pid);
    printw_timer(args);
    napms(1000/2);
  }
}


void timer_loop()
{
  erase();
  PrintTimerArgs args = {.last_type = NULL_TYPE};
  pthread_t print_timer_thread;
  pthread_create(&print_timer_thread, NULL, get_and_printw_timer, &args);
  while (1) {
    if (handle_input_timer(getch())) {
      break;
    }
    if (pid)
      args.timer = get_local_timer_from_pid(pid);
    printw_timer(&args);
  }
  pthread_cancel(print_timer_thread);
}

int main(int argc, char *argv[])
{
  ncurses_initialize_screen();
  while (1) {
    pid = pid_selection_menu();

    timer_loop();
  }
  return EXIT_SUCCESS;
}
