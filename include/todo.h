#ifndef TODO_H__
#define TODOS_START 5
#define MAX_MESSAGE 100
#define MAX_NOTE 100
#define MAX_TODOS 40
#define PRIORITY(x) (x.priority == 0 ? 10: x.priority)
#define prio(x) PRIORITY(x)
typedef struct {
  int file_index, priority;
  char note[MAX_NOTE+1], message[MAX_MESSAGE+1];
  _Bool done;
} Todo;
void Todo_array_bubble_sort_priority(Todo todos[], int size);
void Todo_array_insertion_sort_priority(Todo todos[], int size);
void Todo_array_print_ncurses(Todo todos[], int size);
void Todo_swap(Todo *t1, Todo *t2);
char * Todo_file_path();
void Todo_remove_array_index(Todo todos[], int *size, int index);
unsigned int Todo_array_read_from_file(Todo todos[]);
void ncurses_clear_todos(int todos_size);
void Todo_array_write_to_file(Todo todos[], int todos_size);
void Todo_initialize(Todo *todo);
unsigned int Todo_array_find_index(Todo todo_array[], int size, Todo search);
int Todo_array_search(Todo haystack[], int size, char * needle, int * matching_indexes);
int Todo_array_reorder_index(Todo todos[], int size, int index);
int Todo_decrease_priority(Todo * todo);
int Todo_increase_priority(Todo * todo);
void Todo_array_remove_done(Todo todos[], int *size);
int nc_todo_size(int size);
#endif // TODO_H__
