#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <ncurses.h>

#include "../include/timer.h"
#include "../include/client.h"
#include "../include/todo.h"
#include "../include/ncurses-utils.h"

#define ESC 27
int handle_input_character_common(int ch)
{
  switch (ch) {
    case 'q':
    case 'Q': 
      ncurses_quit();
    break;
    case ESC:
      return 1;
    break;
  }
  return 0;
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

void handle_input_pid_menu(int ch, int *selected_index, pid_t * pid)
{
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
      run_function_on_pid_file_index(remove_potato_pid_file, *selected_index);
      break;
    case 'D':
      run_function_on_pid_file_index(handle_quit, *selected_index);
      break;
    case 'G':
      *selected_index = pids_length_global-1;
      break;
    case '\n':
      *pid = pid_at_index(*selected_index);
    break;
  }
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

  pid_t output = -1;
  draw_pids(*selected_index);

  while (1) {
    int ch = getch();
    handle_input_character_common(ch);
    handle_input_pid_menu(ch, selected_index, &output);
    if (output != -1)
      break;

    draw_pids(*selected_index);
    napms(1000 / 60);
  }
  pthread_cancel(get_pids_thread);
  return output;
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

const char * type_string(TimerType type)
{
  switch(type) {
    case POMODORO_TYPE:
      return "Pomodoro";
    case SHORT_BREAK_TYPE:
      return "Short break";
    case LONG_BREAK_TYPE:
      return "Long break";
  }
}


void fix_selected_index_and_highlight(int * selected_index, int todos_size)
{
  while (*selected_index >= todos_size && *selected_index > 0)
    (*selected_index)--;
  change_color_line(TODOS_START+*selected_index, 1);
}

void Todo_array_write_to_file(Todo todos[], int todos_size);
_Bool todos_changed = FALSE;
Todo get_todo_from_user()
{
  char str[100];
  Todo todo;
  Todo_initialize(&todo);
  enable_delay_and_echo();
  pause_timer_thread();

  mvprintw(MAX_Y, 0, "Todo message: ");
  getstr(str);
  strcpy(todo.message, str);
  clear_line(MAX_Y);

  while (todo.priority < 0 || todo.priority > 9){
    mvprintw(getmaxy(stdscr)-1, 0, "Todo priority [0]: ");
    getstr(str);
    todo.priority = atoi(str);
  }
  clear_line(MAX_Y);

  disable_delay_and_echo();
  unpause_timer_thread();

  return todo;
}
void handle_input_todos_menu(int ch, int *selected_index, int *todos_size, Todo todos[])
{
  switch(ch) {
    case 'k':
      (*selected_index)--;
    break;
    case 'j':
      (*selected_index)++;
    break;
    case 'a': {
      todos_changed = TRUE;
      todos[*todos_size] = get_todo_from_user();
      (*todos_size)++;
      Todo_array_bubble_sort_priority(todos, *todos_size);
      Todo_array_print_ncurses(todos, *todos_size);
      change_color_line(TODOS_START+*selected_index, 1);
      break;
    }
    case 'e': {
      todos_changed = TRUE;
      Todo todo = get_todo_from_user();
      if (!strlen(todo.message))
        strcpy(todo.message, todos[*selected_index].message);
      todos[*selected_index] = todo;
      Todo_array_bubble_sort_priority(todos, *todos_size);
      Todo_array_print_ncurses(todos, *todos_size);
      change_color_line(TODOS_START+*selected_index, 1);
      break;
    }
    case 'd':
      ncurses_clear_todos(*todos_size);
      Todo_remove_array_index(todos,todos_size, *selected_index);
      Todo_array_print_ncurses(todos, *todos_size);
      fix_selected_index_and_highlight(selected_index, *todos_size);
    break;
    case '\n':
      if (*todos_size == 0)
        break;
      attron(COLOR_PAIR(1));
      todos[*selected_index].done = !todos[*selected_index].done;
      if (todos[*selected_index].done)
        mvprintw(TODOS_START+(*selected_index), 1, "x");
      else
        mvprintw(TODOS_START+(*selected_index), 1, " ");
      // mvprintw(getmaxy(stdscr)-1, 0, "%s", todos[*selected_index].message);
      attroff(COLOR_PAIR(1));
      (*selected_index)++;
    break;
    case 'R':
      todos_changed = FALSE;
      *todos_size = Todo_array_read_from_file(todos);
      Todo_array_print_ncurses(todos, *todos_size);
      fix_selected_index_and_highlight(selected_index, *todos_size);
    break;
    case 'w':
      todos_changed = FALSE;
      Todo_array_write_to_file(todos, *todos_size);
      ncurses_clear_todos(*todos_size);
      *todos_size = Todo_array_read_from_file(todos);
      Todo_array_print_ncurses(todos, *todos_size);
      mvprintw(getmaxy(stdscr)-1, 0, "Wrote to file");
      fix_selected_index_and_highlight(selected_index, *todos_size);
    break;
    case 'G':
      *selected_index = *todos_size-1 < 0 ? 0 : *todos_size-1;
    break;
    case 'g':
      *selected_index = 0;
    break;
  }
  if (*selected_index >= *todos_size)
    *selected_index = 0;
  if (*selected_index < 0) {
    *selected_index = *todos_size-1 < 0 ? 0 : *todos_size-1;
  }
}

void *get_and_print_timer(void *arg) {
  Timer *timer = NULL;
  while (1) {
    if (pid)
      timer = get_timer_pid(pid);

    if (timer == NULL)
      timer_thread_paused = 1;

    if (!timer_thread_paused) {
      if (!timer->paused)
        for (int i = 0; i < 3; i++) {
          move(i,0);
          clrtoeol();
        }
      char * time_left_str = Timer_time_left(timer);
      mvprintw(0,0, "Time left: %s", time_left_str);
      mvprintw(1,0, "Pomodoros: %d", timer->pomodoro_count);
      mvprintw(2,0, "Type: %s", type_string(timer->type));
      free(time_left_str);
    }
    napms(1000/3);
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
    case 'J':
    case '-':
    case 'h':
      run_function_on_pid_file_pid(handle_decrease_10sec, pid);
    break;
    case '+':
    case 'K':
    case 'l':
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

#define MAX_TODOS 40
void start_timer_loop_on_thread()
{
  erase();
  mvprintw(TODOS_START-1, 0,"Todos:");
  Todo todos[MAX_TODOS];
  int todos_size = Todo_array_read_from_file(todos);
  Todo_array_print_ncurses(todos, todos_size);

  int selected_index = 0;
  change_color_line(TODOS_START+selected_index, 1);

  while(timer_thread_paused == 0) {
    int ch = getch();
    handle_input_timer(ch);

    if (handle_input_character_common(ch)) {
      if (todos_changed) {
        timer_thread_paused = 1;
        enable_delay_and_echo();
        mvprintw(getmaxy(stdscr)-1, 0,"Todo changes are not saved. Save? [Y/n]: ");
        char ch = getch();
        if (ch != 'n') {
          Todo_array_write_to_file(todos, todos_size);
        }
        timer_thread_paused = 0;
        disable_delay_and_echo();
      }
      mvprintw(12,0, "HELLO");
      break;
    }
    int prior_selected_index = selected_index;
    handle_input_todos_menu(ch, &selected_index, &todos_size, todos);


    if (prior_selected_index != selected_index) {
      change_color_line(TODOS_START+selected_index, 1);
      change_color_line(TODOS_START+prior_selected_index, 0);
    }
    napms(1000 / 60);
  }
}

int main(int argc, char *argv[])
{
  pthread_t timer_thread;
  pthread_create(&timer_thread, NULL, get_and_print_timer, NULL);
  initialize_screen();
  int selected_index = 0;
  while (1) {
    timer_thread_paused = 1;
    pid = 0;
    pid = pid_selection_menu(&selected_index);
    timer_thread_paused = 0;

    start_timer_loop_on_thread();
  }
  return EXIT_SUCCESS;
}
