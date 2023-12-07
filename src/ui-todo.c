#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "../include/utils.h"
#include "../include/ui-todo.h"
#include "../include/ncurses-utils.h"

void TodoMenuArgs_init(TodosMenuArgs * args)
{
  args->search_size = 0;
  args->search_index = -1;
  args->index = 0;
  args->size = Todo_array_read_from_file(args->todos);
  args->start = 0;
}

int nc_todo_size(int size)
{
  return min(getmaxy(stdscr)-TODOS_START-1, size);
}

void nc_todos_print(TodosMenuArgs * args)
{
  for (int i = args->start; i < nc_todo_size(args->size)+args->start; i++) {
    int is_done_ch = args->todos[i].done ? 'x' : ' ';
    mvprintw(TODOS_START+i-args->start, 0,"[%c] [%d] %s\n", is_done_ch, args->todos[i].priority, args->todos[i].message);
  }
}

void nc_todos_clear(int size)
{
  for (int i = TODOS_START; i < TODOS_START+nc_todo_size(size); i++) {
    ncurses_clear_line(i);
  }
}

void fix_index(TodosMenuArgs * args)
{
  if (args->index >= args->size)
    args->index = 0;
  if (args->index < 0) {
    args->index = args->size-1 < 0 ? 0 : args->size-1;
  }
}

void highlight(TodosMenuArgs * args)
{
  ncurses_change_color_line(TODOS_START+args->index-args->start, 1);
}

void set_start(TodosMenuArgs *args)
{
  int nc_size = nc_todo_size(args->size);
  if (args->index >= nc_size && args->index > args->prior_index) {
    args->start = args->index-nc_size+1;
  }
  else if (args->index-args->start < 0) {
    args->start--;
  }
  if (args->index == 0 || args->start < 0 || nc_size == args->size)
    args->start = 0;
}

void set_start_and_highlight(TodosMenuArgs *args)
{
  int prior_start = args->start;
  set_start(args);
  int nc_size = nc_todo_size(args->size);

  if (prior_start != args->start)
    nc_todos_print(args);

  if (args->start == prior_start)
    ncurses_change_color_line(TODOS_START+args->prior_index-prior_start, 0);
  if (args->index-args->start < nc_size)
    highlight(args);
}

void fix_and_highlight_index(TodosMenuArgs *args)
{
  fix_index(args);
  set_start_and_highlight(args);
}

void set_todos_changed(TodosMenuArgs * args, bool input)
{
  args->is_changed = input;
  const char * changed_str = args->is_changed ? "*" : "";
  ncurses_clear_line(TODOS_START-1);
  mvprintw(TODOS_START-1, 0,"Todos (%d)%s:", args->size, changed_str);
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
  if (args->search_index < 0)
    args->search_index = 0;
  args->index = args->search_indexes[args->search_index];
}

void add_to_search(TodosMenuArgs * args, int add)
{
  if (args->search_size <= 0)
    return;
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
  nc_todos_print(args);
  set_start_and_highlight(args);
  set_todos_changed(args, true);
}

void change_priority(TodosMenuArgs *args, int handler(Todo *))
{
  int status = handler(&args->todos[args->index]);
  if (status == EXIT_FAILURE) return;

  args->index = Todo_array_reorder_index(args->todos, args->size,args->index);
  update_changes_todos_nc(args);
}

Todo * selected_todo(TodosMenuArgs * args)
{
  return &args->todos[args->index];
}

void todo_draw_done_char(TodosMenuArgs * args) 
{
  attron(COLOR_PAIR(1));
  const char *is_done_str = selected_todo(args)->done ? "x" : " ";
  mvprintw(TODOS_START+(args->index), 1, "%s", is_done_str);
  attroff(COLOR_PAIR(1));
}

void pause_timer_thread();
void unpause_timer_thread();
int get_todo_from_user(Todo * defaultTodoPtr, Todo *todo)
{
  Todo defaultTodo;
  if (defaultTodoPtr == NULL)
    Todo_initialize(&defaultTodo);
  else
    defaultTodo = *defaultTodoPtr;
  ncurses_ready_for_input();
  pause_timer_thread();

  mvprintw(MAX_Y, 0, "Todo message: ");
  
  int status;
  if (defaultTodoPtr == NULL)
    status = ncurses_getnstr_default_vimode(todo->message, MAX_MESSAGE, NULL);
  else
    status = ncurses_getnstr_default_vimode(todo->message, MAX_MESSAGE, defaultTodo.message);
  ncurses_clear_line(MAX_Y);

  if (strlen(todo->message) && status == EXIT_SUCCESS) {
    do {
      mvprintw(MAX_Y, 0, "Todo priority [%d]: ", defaultTodo.priority);
      clrtoeol();
      char priority_str[2];
      ncurses_getnstr_default_vimode(priority_str, 2, NULL);
      if (!strlen(priority_str))
        todo->priority = defaultTodo.priority;
      else
        todo->priority = atoi(priority_str);
      }
    while (todo->priority < 0 || todo->priority > 9);
  } else {
    todo->priority = defaultTodo.priority;
  }
  todo->done = defaultTodo.done;
  todo->file_index = defaultTodo.file_index;

  ncurses_clear_line(MAX_Y);

  ncurses_unready_for_input();
  unpause_timer_thread();

  return status;
}


void nc_todos_input(TodosMenuArgs *args)
{
  args->prior_index = args->index;
  switch(args->ch) {
    case KEY_RESIZE:
      nc_todos_print(args);
      ncurses_clear_line(MAX_Y);
      fix_and_highlight_index(args);
    break;
    case 'k':
      args->index--;
    break;
    case 'j':
      args->index++;
    break;
    case 'G':
      args->index = args->size-1;
    break;
    case 'g':
      args->index = 0;
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
      fix_and_highlight_index(args);
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
      if (get_todo_from_user(NULL, &args->todos[args->size])) return;

      args->size++;
      args->index = Todo_array_reorder_index(args->todos, args->size, args->size-1);
      update_changes_todos_nc(args);
      break;
    }
    case 'e': {
      if (get_todo_from_user(selected_todo(args), selected_todo(args))) return;

      args->index = Todo_array_reorder_index(args->todos, args->size,args->index);
      update_changes_todos_nc(args);
      break;
    }
    case 'd':
      nc_todos_clear(args->size);
      Todo_remove_array_index(args->todos,&args->size, args->index);
      update_changes_todos_nc(args);
    break;
    case '\n':
      if (args->size == 0)
        break;
      Todo_toggle_done(selected_todo(args));
      todo_draw_done_char(args);

      set_todos_changed(args, true);
      args->index++;
    break;
    case 'R':
      nc_todos_clear(args->size);
      args->size = Todo_array_read_from_file(args->todos);
      set_todos_changed(args, false);
      nc_todos_print(args);
      mvprintw(MAX_Y, 0, "Read from file");
    break;
    case 'w':
      Todo_array_write_to_file(args->todos, args->size);
      Todo_array_remove_done(args->todos, &args->size);

      nc_todos_clear(args->size);
      set_todos_changed(args, false);
      nc_todos_print(args);
      mvprintw(MAX_Y, 0, "Wrote to file");
    break;
    case ':':
      ncurses_clear_line(MAX_Y);
    break;
  }
  fix_and_highlight_index(args);
}
