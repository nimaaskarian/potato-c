#ifndef TODO_H__
#define TODOS_START 5
typedef struct {
  int file_index, priority;
  char note[4096], message[4096];
  _Bool done;
} Todo;
void Todo_array_bubble_sort_priority(Todo todos[], int size);
void Todo_array_print_ncurses(Todo todos[], int size);
void Todo_swap(Todo *t1, Todo *t2);
char * Todo_file_path();
void Todo_remove_array_index(Todo todos[], int *size, int index);
unsigned int Todo_array_read_from_file(Todo todos[]);
void ncurses_clear_todos(int todos_size);
void Todo_array_write_to_file(Todo todos[], int todos_size);
void Todo_initialize(Todo *todo);
#endif // TODO_H__
