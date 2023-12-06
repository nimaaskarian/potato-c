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
#include "../include/todo.h"
#include "../include/ncurses-utils.h"

typedef struct {
  int ch;
  int index;
  int size;
  // int nc_size;
  Todo todos[MAX_TODOS];
  int search_indexes[MAX_TODOS];
  int search_size;
  int search_index;
}TodosMenuArgs;

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


void fix_and_highlight_index(TodosMenuArgs *args)
{
  while (args->index >= nc_todo_size(args->size) && args->index > 0)
    args->index--;
  ncurses_change_color_line(TODOS_START+args->index, 1);
}

_Bool todos_changed;
void set_todos_changed(_Bool input, int count)
{
  todos_changed = input;
  if (input)
    mvprintw(TODOS_START-1, 0,"Todos (%d)*:", count);
  else
  {
    ncurses_clear_line(TODOS_START-1);
    mvprintw(TODOS_START-1, 0,"Todos (%d):", count);
  }
}

Todo get_todo_from_user(Todo * defaultTodoPtr, int * status_output)
{
  Todo todo, defaultTodo;
  if (defaultTodoPtr == NULL)
    Todo_initialize(&defaultTodo);
  else
    defaultTodo = *defaultTodoPtr;
  ncurses_ready_for_input();
  pause_timer_thread();

  mvprintw(MAX_Y, 0, "Todo message: ");
  
  int status;
  if (defaultTodoPtr == NULL)
    status = ncurses_getnstr_default_vimode(todo.message, MAX_MESSAGE, NULL);
  else
    status = ncurses_getnstr_default_vimode(todo.message, MAX_MESSAGE, defaultTodo.message);
  *status_output = status;
  ncurses_clear_line(MAX_Y);

  if (strlen(todo.message) && status == EXIT_SUCCESS) {
    do {
      mvprintw(MAX_Y, 0, "Todo priority [%d]: ", defaultTodo.priority);
      clrtoeol();
      char priority_str[2];
      ncurses_getnstr_default_vimode(priority_str, 2, NULL);
      if (!strlen(priority_str))
        todo.priority = defaultTodo.priority;
      else
        todo.priority = atoi(priority_str);
      }
    while (todo.priority < 0 || todo.priority > 9);
  } else {
    todo.priority = defaultTodo.priority;
  }
  todo.done = defaultTodo.done;
  todo.file_index = defaultTodo.file_index;

  ncurses_clear_line(MAX_Y);

  ncurses_unready_for_input();
  unpause_timer_thread();

  return todo;
}

void go_to_next_search(TodosMenuArgs *args)
{
  if (args->search_size == 0) {
    args->search_index = -1;
    return;
  }

  while (args->search_indexes[args->search_index] < args->index) {
    if (args->search_index >= args->search_size) {
      args->search_index = 0;
      break;
    }
    args->search_index++;
  }
  args->index = args->search_indexes[args->search_index];
  fix_and_highlight_index(args);
}

void add_to_search(TodosMenuArgs * args, int add)
{
  args->search_index+=add;
  if (args->search_index >= args->search_size) {
    args->search_index = 0;
  }
  if (args->search_index < 0) {
    args->search_index = args->search_size-1;
  }
  args->index = args->search_indexes[args->search_index];
}

void update_changes_todos_nc(TodosMenuArgs *args)
{
  Todo_array_print_ncurses(args->todos, args->size);
  ncurses_change_color_line(TODOS_START+args->index, 1);
  set_todos_changed(TRUE, args->size);
}

void change_priority(TodosMenuArgs *args, int handler(Todo *))
{
  int status = handler(&args->todos[args->index]);
  if (status == EXIT_FAILURE) return;

  args->index = Todo_array_reorder_index(args->todos, args->size,args->index);
  update_changes_todos_nc(args);
}

void todo_toggle_done(TodosMenuArgs * args) 
{
  attron(COLOR_PAIR(1));
  if (args->todos[args->index].done) {
    args->todos[args->index].done = 0;
    mvprintw(TODOS_START+(args->index), 1, " ");
  } else {
    args->todos[args->index].done = 1;
    mvprintw(TODOS_START+(args->index), 1, "x");
  }
  attroff(COLOR_PAIR(1));
}

void fix_index(TodosMenuArgs * args)
{
  if (args->index < 0)
    args->index = 0;
  int nc_size = nc_todo_size(args->size);
  if (args->index >= nc_size)
    args->index = 0;
  if (args->index < 0) {
    args->index = nc_size-1 < 0 ? 0 : nc_size-1;
  }
}

void handle_input_todos_menu(TodosMenuArgs *args)
{
  switch(args->ch) {
    case KEY_RESIZE:
      Todo_array_print_ncurses(args->todos, args->size);
      fix_and_highlight_index(args);
    break;
    case 'k':
      args->index--;
    break;
    case 'j':
      args->index++;
    break;
    case '/':{
      char search_term[MAX_MESSAGE+1];
      mvprintw(MAX_Y, 0, "Search: ");
      ncurses_ready_for_input();
      ncurses_getnstr_default_vimode(search_term, MAX_MESSAGE, NULL);
      ncurses_unready_for_input();
      ncurses_clear_line(MAX_Y);
      args->search_size = Todo_array_search(args->todos, args->size, search_term,args->search_indexes);
      go_to_next_search(args);
    }
    break;
    case 'n':
      add_to_search(args, 1);
    break;
    case 'N':
      add_to_search(args, -1);
    break;
    case 'J':
      change_priority(args, Todo_decrease_priority);
    break;
    case 'K':
      change_priority(args, Todo_increase_priority);
    break;
    case 'a': {
      int status;
      args->todos[args->size] = get_todo_from_user(NULL, &status);
      if (status == EXIT_FAILURE) return;

      args->size++;
      args->index = Todo_array_reorder_index(args->todos, args->size, args->size-1);
      update_changes_todos_nc(args);
      break;
    }
    case 'e': {
      int status;
      args->todos[args->index] = get_todo_from_user(&args->todos[args->index], &status);
      if (status == EXIT_FAILURE) return;

      args->index = Todo_array_reorder_index(args->todos, args->size,args->index);
      update_changes_todos_nc(args);
      break;
    }
    case 'd':
      ncurses_clear_todos(args->size);
      Todo_remove_array_index(args->todos,&args->size, args->index);
      update_changes_todos_nc(args);
    break;
    case '\n':
      if (args->size == 0)
        break;
      todo_toggle_done(args);

      set_todos_changed(TRUE, args->size);
      args->index++;
    break;
    case 'R':
      ncurses_clear_todos(args->size);
      args->size = Todo_array_read_from_file(args->todos);
      set_todos_changed(FALSE, args->size);
      Todo_array_print_ncurses(args->todos, args->size);
      mvprintw(MAX_Y, 0, "Read from file");
      fix_and_highlight_index(args);
    break;
    case 'w':
      Todo_array_write_to_file(args->todos, args->size);
      Todo_array_remove_done(args->todos, &args->size);

      ncurses_clear_todos(args->size);
      set_todos_changed(FALSE, args->size);
      Todo_array_print_ncurses(args->todos, args->size);
      mvprintw(MAX_Y, 0, "Wrote to file");
      fix_and_highlight_index(args);
    break;
    case ':':
      ncurses_clear_line(MAX_Y);
    break;
    case 'G':
      args->index = args->size-1;
    break;
    case 'g':
      args->index = 0;
    break;
  }
  fix_index(args);
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

void start_timer_loop_on_thread()
{
  erase();
  pthread_t timer_thread;
  pthread_create(&timer_thread, NULL, get_and_print_local_timer, NULL);

  TodosMenuArgs todo_menu_args;
  todo_menu_args.search_index = -1;
  todo_menu_args.size = Todo_array_read_from_file(todo_menu_args.todos);
  todo_menu_args.search_size = 0;
  set_todos_changed(FALSE, todo_menu_args.size);

  Todo_array_print_ncurses(todo_menu_args.todos, todo_menu_args.size);

  // int selected_index = 0;
  todo_menu_args.index = 0;
  ncurses_change_color_line(TODOS_START+todo_menu_args.index, 1);

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
          Todo_array_write_to_file(todo_menu_args.todos, todo_menu_args.size);
        }
        timer_thread_paused = 0;
        ncurses_unready_for_input();
      }
      break;
    }
    int prior_selected_index = todo_menu_args.index;
    todo_menu_args.ch = ch;
    handle_input_todos_menu(&todo_menu_args);


    if (prior_selected_index != todo_menu_args.index) {
      ncurses_change_color_line(TODOS_START+todo_menu_args.index, 1);
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
    pause_timer_thread();
    pid = pid_selection_menu(&selected_index);
    unpause_timer_thread();

    start_timer_loop_on_thread();
  }
  return EXIT_SUCCESS;
}
