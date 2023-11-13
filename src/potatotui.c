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

pid_t handle_input_pid_menu(int ch, int *selected_index)
{
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
      run_function_on_pid_file_index(remove_potato_pid_file, *selected_index);
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
    handle_input_character_common(ch);
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
  ncurses_change_color_line(TODOS_START+*selected_index, 1);
}

_Bool todos_changed = FALSE;
Todo get_todo_from_user(int defaultPriority)
{
  #define MAX_TODO 100
  char str[MAX_TODO];
  Todo todo;
  Todo_initialize(&todo);
  ncurses_ready_for_input();
  pause_timer_thread();

  mvprintw(MAX_Y, 0, "Todo message: ");
  
  getnstr(str, MAX_TODO);
  if (strlen(str))
    strcpy(todo.message, str);
  ncurses_clear_line(MAX_Y);

  if (strlen(str))
    while (todo.priority < 0 || todo.priority > 9){
      mvprintw(MAX_Y, 0, "Todo priority [0]: ");
      clrtoeol();
      getnstr(str, 2);
      if (!strlen(str)) {
        todo.priority = defaultPriority;
        break;
      }
      todo.priority = atoi(str);
    }
  ncurses_clear_line(MAX_Y);

  ncurses_unready_for_input();
  unpause_timer_thread();

  return todo;
}

int get_todos_scroll_size(int real_todos_size)
{
  return min(getmaxy(stdscr)-TODOS_START-1, real_todos_size);
}

void handle_input_todos_menu(int ch, int *selected_index, int *real_todos_size, Todo todos[], int *nc_todos_size)
{
  switch(ch) {
    case KEY_RESIZE:
      *nc_todos_size = get_todos_scroll_size(*real_todos_size);
      Todo_array_print_ncurses(todos, *nc_todos_size);
      fix_selected_index_and_highlight(selected_index, *nc_todos_size);
    break;
    case 'k':
      (*selected_index)--;
    break;
    case 'j':
      (*selected_index)++;
    break;
    case 'J': {
        if (todos[*selected_index].priority == 0)
          break;
        if (todos[*selected_index].priority == 9)
          todos[*selected_index].priority = 0;
        else
          todos[*selected_index].priority++;

        Todo current_todo = todos[*selected_index];
        Todo_array_bubble_sort_priority(todos, *nc_todos_size);
        *selected_index = Todo_array_find_index(todos, *nc_todos_size, current_todo);
        Todo_array_print_ncurses(todos, *nc_todos_size);
        ncurses_change_color_line(TODOS_START+*selected_index, 1);
        break;
      }
    case 'K': {
        if (todos[*selected_index].priority == 1)
          break;
        if (todos[*selected_index].priority == 0)
          todos[*selected_index].priority = 9;
        else
          todos[*selected_index].priority--;
          
        Todo current_todo = todos[*selected_index];
        Todo_array_bubble_sort_priority(todos, *nc_todos_size);
        *selected_index = Todo_array_find_index(todos, *nc_todos_size, current_todo);
        Todo_array_print_ncurses(todos, *nc_todos_size);
        ncurses_change_color_line(TODOS_START+*selected_index, 1);
        break;
      }
    case 'a': {
      todos[*nc_todos_size] = get_todo_from_user(0);
      if (!strlen(todos[*nc_todos_size].message))
        break;
      todos_changed = TRUE;
      (*nc_todos_size)++;
      Todo_array_bubble_sort_priority(todos, *nc_todos_size);
      Todo_array_print_ncurses(todos, *nc_todos_size);
      ncurses_change_color_line(TODOS_START+*selected_index, 1);
      break;
    }
    case 'e': {
      Todo todo = get_todo_from_user(todos[*selected_index].priority);
      if (!strlen(todo.message))
        break;
      todos_changed = TRUE;
      strcpy(todo.note, todos[*selected_index].note);

      if (!strlen(todo.message))
        strcpy(todo.message, todos[*selected_index].message);

      todos[*selected_index] = todo;
      Todo_array_bubble_sort_priority(todos, *nc_todos_size);
      Todo_array_print_ncurses(todos, *nc_todos_size);
      ncurses_change_color_line(TODOS_START+*selected_index, 1);
      break;
    }
    case 'd':
      ncurses_clear_todos(*nc_todos_size);
      Todo_remove_array_index(todos,nc_todos_size, *selected_index);
      Todo_array_print_ncurses(todos, *nc_todos_size);
      fix_selected_index_and_highlight(selected_index, *nc_todos_size);
    break;
    case '\n':
      if (*nc_todos_size == 0)
        break;
      attron(COLOR_PAIR(1));
      todos[*selected_index].done = !todos[*selected_index].done;
      if (todos[*selected_index].done)
        mvprintw(TODOS_START+(*selected_index), 1, "x");
      else
        mvprintw(TODOS_START+(*selected_index), 1, " ");

      attroff(COLOR_PAIR(1));
      (*selected_index)++;
    break;
    case 'R':
      todos_changed = FALSE;
      ncurses_clear_lines_from_to(TODOS_START, TODOS_START+*nc_todos_size);
      *real_todos_size = Todo_array_read_from_file(todos);
      *nc_todos_size = get_todos_scroll_size(*real_todos_size);
      Todo_array_print_ncurses(todos, *nc_todos_size);
      mvprintw(MAX_Y, 0, "Read from file");
      fix_selected_index_and_highlight(selected_index, *nc_todos_size);
      refresh();
      ncurses_sleep_and_clear_line(2, MAX_Y);
    break;
    case 'w':
      todos_changed = FALSE;
      Todo_array_write_to_file(todos, *real_todos_size);
      ncurses_clear_todos(*nc_todos_size);
      *real_todos_size = Todo_array_read_from_file(todos);
      *nc_todos_size = get_todos_scroll_size(*real_todos_size);
      Todo_array_print_ncurses(todos, *nc_todos_size);
      mvprintw(MAX_Y, 0, "Wrote to file");
      fix_selected_index_and_highlight(selected_index, *nc_todos_size);
      refresh();
      ncurses_sleep_and_clear_line(2, MAX_Y);
    break;
    case 'G':
      *selected_index = *nc_todos_size-1 < 0 ? 0 : *nc_todos_size-1;
    break;
    case 'g':
      *selected_index = 0;
    break;
  }
  if (*selected_index >= *nc_todos_size)
    *selected_index = 0;
  if (*selected_index < 0) {
    *selected_index = *nc_todos_size-1 < 0 ? 0 : *nc_todos_size-1;
  }
}

void *get_and_print_timer(void *arg) {
  Timer timer;
  while (1) {
    if (pid)
      timer = get_timer_pid(pid);

    if (timer.type == NULL_TYPE) {
      pause_timer_thread();
      break;
    }

    if (!timer_thread_paused) {
      if (!timer.paused)
        ncurses_clear_lines_from_to(0,3);
      char * time_left_str = Timer_time_left(&timer);
      mvprintw(0,0, "Time left: %s", time_left_str);
      mvprintw(1,0, "Pomodoros: %d", timer.pomodoro_count);
      mvprintw(2,0, "Type: %s", type_string(timer.type));
      free(time_left_str);
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
    case '-':
    case 'l':
      run_function_on_pid_file_pid(handle_decrease_10sec, pid);
    break;
    case '+':
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

#define MAX_TODOS 40
void start_timer_loop_on_thread()
{
  erase();
  pthread_t timer_thread;
  pthread_create(&timer_thread, NULL, get_and_print_timer, NULL);

  mvprintw(TODOS_START-1, 0,"Todos:");
  Todo todos[MAX_TODOS];
  int real_todos_size = Todo_array_read_from_file(todos);
  int todos_size = get_todos_scroll_size(real_todos_size);

  Todo_array_print_ncurses(todos, todos_size);

  int selected_index = 0;
  ncurses_change_color_line(TODOS_START+selected_index, 1);

  while(timer_thread_paused == 0) {
    int ch = getch();
    handle_input_timer(ch);

    if (handle_input_character_common(ch)) {
      if (todos_changed) {
        timer_thread_paused = 1;
        ncurses_ready_for_input();
        mvprintw(MAX_Y, 0,"Todo changes are not saved. Save? [Y/n]: ");
        char ch = getch();
        if (ch != 'n') {
          Todo_array_write_to_file(todos, real_todos_size);
        }
        timer_thread_paused = 0;
        ncurses_unready_for_input();
      }
      break;
    }
    int prior_selected_index = selected_index;
    handle_input_todos_menu(ch, &selected_index, &real_todos_size, todos, &todos_size);


    if (prior_selected_index != selected_index) {
      ncurses_change_color_line(TODOS_START+selected_index, 1);
      ncurses_change_color_line(TODOS_START+prior_selected_index, 0);
    }
    napms(1000 / 60);
  }
  pthread_cancel(timer_thread);
}

int main(int argc, char *argv[])
{
  ncurses_initialize_screen();
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
