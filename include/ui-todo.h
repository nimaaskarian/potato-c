#ifndef UI_TODO_H__
#include "../include/todo.h"

typedef struct {
  int ch;
  int index, prior_index;
  int size;
  // int nc_size;
  Todo todos[MAX_TODOS];
  int search_indexes[MAX_TODOS];
  int search_size;
  int search_index;
  bool is_changed;
} TodosMenuArgs;

void nc_todos_print(Todo todos[], int size);
void nc_todos_clear(int todos_size);
int nc_todo_size(int size);
void nc_todos_input(TodosMenuArgs *args);
void set_todos_changed(TodosMenuArgs * args, bool input);
void TodoMenuArgs_init(TodosMenuArgs * args);

#endif // UI_TODO_H__
