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
#include "../include/ui-todo.h"
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

int pids_length_global;
void check_pids_length_and_set_selected(int *selected_index)
{
  if (*selected_index < 0) {
    *selected_index = pids_length_global ? pids_length_global-1 : 0;
    return;
  }

  if (*selected_index >= pids_length_global)  {
    *selected_index = 0;
  }
}

pid_t handle_input_pid_menu(int ch, int *selected_index)
{
  if (pids_length_global == 1)
    return pid_at_index(0);
  pid_t pid = 0;
  switch (ch) {
    case 'j':
      (*selected_index)++;
      check_pids_length_and_set_selected(selected_index);
      break;
    case 'k':
      (*selected_index)--;
      check_pids_length_and_set_selected(selected_index);
      break;
    case 'g':
      *selected_index = 0;
      break;
    case 'c':
      run_function_on_pid_file_index(handle_remove_pid, *selected_index);
      pids_length_global--;
      break;
    case 'D':
      run_function_on_pid_file_index(handle_quit, *selected_index);
      pids_length_global--;
      break;
    case 'G':
      *selected_index = pids_length_global-1;
      break;
    case '\n':
      pid = pid_at_index(*selected_index);
    break;
  }
  if (*selected_index >= pids_length_global)
    *selected_index = pids_length_global ? pids_length_global - 1 : 0;
  return pid;
}

void draw_pids(int selected_index)
{
  erase();
  if (pids_length_global) {
    run_function_on_pid_file_index(printw_pid,EVERY_MEMBER);
    attron(COLOR_PAIR(1));
    run_function_on_pid_file_index(printw_pid,selected_index);
    attroff(COLOR_PAIR(1));
  } else {
    mvprintw(0,0, "No daemons found right now.");
  }
}

void * get_pids_length_sleep(void* args)
{
  while (1) {
    pids_length_global = get_pids_length();
    sleep(2);
  }
  pthread_exit(EXIT_SUCCESS);
}

pid_t pid_selection_menu(int *selected_index)
{
  pthread_t get_pids_thread;
  pthread_create(&get_pids_thread, NULL, get_pids_length_sleep, NULL);

  pid_t selected_pid;
  draw_pids(*selected_index);

  while (1) {
    int ch = getch();
    if (input_quit_menu(ch) == QUIT_QUIT) ncurses_quit();
    selected_pid = handle_input_pid_menu(ch, selected_index);

    if (selected_pid)
      break;

    draw_pids(*selected_index);
    napms(1000 / 60);
  }
  pthread_cancel(get_pids_thread);
  return selected_pid;
}

pid_t pid;
_Bool timer_thread_paused = 0;
void pause_timer_thread()
{
  timer_thread_paused = 1;
}

void unpause_timer_thread()
{
  timer_thread_paused = 0;
}

const char * get_type_string(TimerType type)
{
  switch(type) {
    case POMODORO_TYPE:
      return "Pomodoro";
    case SHORT_BREAK_TYPE:
      return "Short break";
    case LONG_BREAK_TYPE:
      return "Long break";
    case NULL_TYPE:
      return "";
  }
}

void printw_timer_return_last_type(Timer timer, TimerType *last_type)
{
  if (!timer.paused || timer.type != *last_type) {
    ncurses_clear_lines_from_to(0,3);
    *last_type = timer.type;
  }
  char * time_left_str = Timer_time_left(&timer);
  mvprintw(0,0, "Time left: %s", time_left_str);
  mvprintw(1,0, "Pomodoros: %d", timer.pomodoro_count);
  const char * type_string = get_type_string(timer.type);
  mvprintw(2,0, "Type: %s", type_string);
  free(time_left_str);
}

void *get_and_print_local_timer(void *arg) {
  TimerType last_type = NULL_TYPE;
  Timer timer;
  while (1) {
    if (pid)
      timer = get_local_timer_from_pid(pid);

    if (timer.type == NULL_TYPE) {
      pause_timer_thread();
      break;
    }

    if (!timer_thread_paused) {
      printw_timer_return_last_type(timer, &last_type);
    }
    napms(1000/2);
  }
	pthread_exit(EXIT_SUCCESS);
}

void handle_input_timer(int ch)
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
  }
}

int handle_todos_quit(TodosMenuArgs * args, enum QUIT_MENU quit_status)
{
  if (quit_status == QUIT_NONE) 
    return 0;

  if (args->is_changed) {
    pause_timer_thread();
    if (getch_mvprintf(MAX_Y, 0, "Todo changes are not saved. Save? [y/N]: ") == 'y') {
      Todo_array_write_to_file(args->todos, args->size);
    }
    unpause_timer_thread();
  }
  switch (quit_status) {
    case QUIT_ESC:
      return 1;
    case QUIT_QUIT:
      ncurses_quit();
  }
  return 0;
}

void start_timer_loop_on_thread()
{
  erase();
  pthread_t timer_thread;
  pthread_create(&timer_thread, NULL, get_and_print_local_timer, NULL);

  TodosMenuArgs todo_args;
  TodoMenuArgs_init(&todo_args);
  set_todos_changed(&todo_args, false);

  nc_todos_print(&todo_args);
  ncurses_change_color_line(TODOS_START+todo_args.index, 1);

  while(timer_thread_paused == 0) {
    int ch = getch();
    todo_args.ch = ch;
    if (handle_todos_quit(&todo_args, input_quit_menu(ch)))
      break;

    handle_input_timer(ch);
    nodelay(stdscr, FALSE);
    nc_todos_input(&todo_args);
    nodelay(stdscr, TRUE);

    napms(1000 / 60);
  }
  pthread_cancel(timer_thread);
}

int main(int argc, char *argv[])
{
  ncurses_initialize_screen();
  int selected_index = 0;
  while (1) {
    pause_timer_thread();
    pid = pid_selection_menu(&selected_index);
    unpause_timer_thread();

    start_timer_loop_on_thread();
  }
  return EXIT_SUCCESS;
}
