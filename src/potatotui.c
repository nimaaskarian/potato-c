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


void fix_selected_index_and_highlight(int * selected_index, int todos_size)
{
  while (*selected_index >= todos_size && *selected_index > 0)
    (*selected_index)--;
  ncurses_change_color_line(TODOS_START+*selected_index, 1);
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

int get_todos_scroll_size(int real_todos_size)
{
  return min(getmaxy(stdscr)-TODOS_START-1, real_todos_size);
}

#define MAX_TODOS 40
typedef struct {
  int ch;
  int selected_index;
  int real_todos_size;
  int nc_todos_size;
  Todo todos[MAX_TODOS];
  int searched_todos[MAX_TODOS];
  int searched_todos_size;
  int searched_todos_index;
}TodosMenuArgs;

void handle_input_todos_menu(TodosMenuArgs *args)
{
  switch(args->ch) {
    case KEY_RESIZE:
      args->nc_todos_size = get_todos_scroll_size(args->real_todos_size);
      Todo_array_print_ncurses(args->todos, args->nc_todos_size);
      fix_selected_index_and_highlight(&args->selected_index, args->nc_todos_size);
    break;
    case 'k':
      (args->selected_index)--;
    break;
    case 'j':
      (args->selected_index)++;
    break;
    case '/':{
      char search_term[MAX_MESSAGE+1];
      mvprintw(MAX_Y, 0, "Search: ");
      ncurses_ready_for_input();
      ncurses_getnstr_default_vimode(search_term, MAX_MESSAGE, NULL);
      ncurses_unready_for_input();
      ncurses_clear_line(MAX_Y);
      args->searched_todos_size = 
        Todo_array_search(args->todos, args->real_todos_size,
                          search_term,args->searched_todos);
      if (args->searched_todos_size != 0) {
        args->searched_todos_index = 0;
        while (args->searched_todos[args->searched_todos_index] < args->selected_index) {
          if (args->searched_todos_index >= args->searched_todos_size) {
            args->searched_todos_index = 0;
            break;
          }
          args->searched_todos_index++;
        }
        args->selected_index = args->searched_todos[args->searched_todos_index];
      } else {
        args->searched_todos_index = -1;
      }
    }
    break;
    case 'n':
      if (args->searched_todos_size == 0)
        break;
      args->searched_todos_index++;
      if (args->searched_todos_index >= args->searched_todos_size) {
        args->searched_todos_index = 0;
      }
      args->selected_index = args->searched_todos[args->searched_todos_index];
    break;
    case 'N':
      if (args->searched_todos_size == 0)
        break;
      args->searched_todos_index--;
      if (args->searched_todos_index < 0) {
        args->searched_todos_index = args->searched_todos_size-1;
      }
      args->selected_index = args->searched_todos[args->searched_todos_index];
    break;
    case 'J': {
        if (args->todos[args->selected_index].priority == 0)
          break;
        if (args->todos[args->selected_index].priority == 9)
          args->todos[args->selected_index].priority = 0;
        else
          args->todos[args->selected_index].priority++;

        Todo current_todo = args->todos[args->selected_index];
        Todo_array_insertion_sort_priority(args->todos, args->nc_todos_size);
        // Todo_array_bubble_sort_priority(args->todos, *nc_todos_size);
        args->selected_index = Todo_array_find_index(args->todos, args->nc_todos_size, current_todo);
        Todo_array_print_ncurses(args->todos, args->nc_todos_size);
        ncurses_change_color_line(TODOS_START+args->selected_index, 1);
        set_todos_changed(TRUE, args->real_todos_size);
        break;
      }
    case 'K': {
        if (args->todos[args->selected_index].priority == 1)
          break;
        if (args->todos[args->selected_index].priority == 0)
          args->todos[args->selected_index].priority = 9;
        else
          args->todos[args->selected_index].priority--;
          
        Todo current_todo = args->todos[args->selected_index];
        Todo_array_insertion_sort_priority(args->todos, args->nc_todos_size);
        // Todo_array_bubble_sort_priority(args->todos, args->nc_todos_size);
        args->selected_index = Todo_array_find_index(args->todos, args->nc_todos_size, current_todo);
        Todo_array_print_ncurses(args->todos, args->nc_todos_size);
        ncurses_change_color_line(TODOS_START+args->selected_index, 1);
        set_todos_changed(TRUE, args->real_todos_size);
        break;
      }
    case 'a': {
      int status;
      args->todos[args->real_todos_size] = get_todo_from_user(NULL, &status);
      if (status != EXIT_SUCCESS)
        break;
      args->real_todos_size++;
      set_todos_changed(TRUE, args->real_todos_size);
      Todo_array_insertion_sort_priority(args->todos, args->real_todos_size);
      // Todo_array_bubble_sort_priority(args->todos, *real_todos_size);
      Todo_array_print_ncurses(args->todos, args->real_todos_size);
      ncurses_change_color_line(TODOS_START+args->selected_index, 1);
      break;
    }
    case 'e': {
      int status;
      Todo todo = get_todo_from_user(&args->todos[args->selected_index], &status);
      if (status != EXIT_SUCCESS)
        break;
      strcpy(todo.note, args->todos[args->selected_index].note);

      if (!strlen(todo.message))
        strcpy(todo.message, args->todos[args->selected_index].message);

      args->todos[args->selected_index] = todo;
      Todo_array_insertion_sort_priority(args->todos, args->nc_todos_size);
      // Todo_array_bubble_sort_priority(args->todos, args->nc_todos_size);
      Todo_array_print_ncurses(args->todos, args->nc_todos_size);
      ncurses_change_color_line(TODOS_START+args->selected_index, 1);
      set_todos_changed(TRUE, args->real_todos_size);
      break;
    }
    case 'd':
      ncurses_clear_todos(args->nc_todos_size);
      Todo_remove_array_index(args->todos,&args->real_todos_size, args->selected_index);
      args->nc_todos_size = get_todos_scroll_size(args->real_todos_size);
      Todo_array_print_ncurses(args->todos, args->nc_todos_size);
      fix_selected_index_and_highlight(&args->selected_index, args->nc_todos_size);
    break;
    case '\n':
      if (args->nc_todos_size == 0)
        break;
      attron(COLOR_PAIR(1));
      // args->todos[args->selected_index].done = !args->todos[args->selected_index].done;
      if (args->todos[args->selected_index].done) {
        args->todos[args->selected_index].done = 0;
        mvprintw(TODOS_START+(args->selected_index), 1, " ");
      } else {
        args->todos[args->selected_index].done = 1;
        mvprintw(TODOS_START+(args->selected_index), 1, "x");
      }

      attroff(COLOR_PAIR(1));
      set_todos_changed(TRUE, args->real_todos_size);
      args->selected_index++;
    break;
    case 'R':
      set_todos_changed(FALSE, args->real_todos_size);
      ncurses_clear_lines_from_to(TODOS_START, TODOS_START+args->nc_todos_size);
      args->real_todos_size = Todo_array_read_from_file(args->todos);
      args->nc_todos_size = get_todos_scroll_size(args->real_todos_size);
      Todo_array_print_ncurses(args->todos, args->nc_todos_size);
      mvprintw(MAX_Y, 0, "Read from file");
      fix_selected_index_and_highlight(&args->selected_index, args->nc_todos_size);
      refresh();
      ncurses_sleep_and_clear_line(2, MAX_Y);
    break;
    case 'w':
      set_todos_changed(FALSE, args->real_todos_size);
      Todo_array_write_to_file(args->todos, args->real_todos_size);
      ncurses_clear_todos(args->nc_todos_size);
      args->real_todos_size = Todo_array_read_from_file(args->todos);
      args->nc_todos_size = get_todos_scroll_size(args->real_todos_size);
      Todo_array_print_ncurses(args->todos, args->nc_todos_size);
      mvprintw(MAX_Y, 0, "Wrote to file");
      fix_selected_index_and_highlight(&args->selected_index, args->nc_todos_size);
    break;
    case ':':
      ncurses_clear_line(MAX_Y);
    break;
    case 'G':
      args->selected_index = args->nc_todos_size-1 < 0 ? 0 : args->nc_todos_size-1;
    break;
    case 'g':
      args->selected_index = 0;
    break;
  }
  args->nc_todos_size = get_todos_scroll_size(args->real_todos_size);
  if (args->selected_index >= args->nc_todos_size)
    args->selected_index = 0;
  if (args->selected_index < 0) {
    args->selected_index = args->nc_todos_size-1 < 0 ? 0 : args->nc_todos_size-1;
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
      const char * type_string = get_type_string(timer.type);
      mvprintw(2,0, "Type: %s", type_string);
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

void start_timer_loop_on_thread()
{
  erase();
  pthread_t timer_thread;
  pthread_create(&timer_thread, NULL, get_and_print_timer, NULL);

  TodosMenuArgs todo_menu_args;
  todo_menu_args.searched_todos_index = -1;
  // int real_todos_size = Todo_array_read_from_file(todo_menu_args.todos);
  // int nc_todos_size = get_todos_scroll_size(real_todos_size);
  todo_menu_args.real_todos_size = Todo_array_read_from_file(todo_menu_args.todos);
  todo_menu_args.nc_todos_size = get_todos_scroll_size(todo_menu_args.real_todos_size);;
  todo_menu_args.searched_todos_size = 0;
  set_todos_changed(FALSE, todo_menu_args.real_todos_size);

  Todo_array_print_ncurses(todo_menu_args.todos, todo_menu_args.nc_todos_size);

  // int selected_index = 0;
  todo_menu_args.selected_index = 0;
  ncurses_change_color_line(TODOS_START+todo_menu_args.selected_index, 1);

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
          Todo_array_write_to_file(todo_menu_args.todos, todo_menu_args.real_todos_size);
        }
        timer_thread_paused = 0;
        ncurses_unready_for_input();
      }
      break;
    }
    int prior_selected_index = todo_menu_args.selected_index;
    todo_menu_args.ch = ch;
    handle_input_todos_menu(&todo_menu_args);


    if (prior_selected_index != todo_menu_args.selected_index) {
      ncurses_change_color_line(TODOS_START+todo_menu_args.selected_index, 1);
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
