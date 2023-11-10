#include <math.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <ncurses.h>

#include "../include/todo.h"

char * Todo_file_path()
{
  const char* home = getenv("HOME");
  char *todo_path = malloc(4096*sizeof(char));
  snprintf(todo_path, 4096, "%s/.local/share/calcurse/todo", home);

  return todo_path;
}
 
void Todo_remove_array_index(Todo todos[], int *size, int index)
{
  int i;
  for(i = index; i < *size; i++) todos[i] = todos[i + 1];
  if (*size)
    (*size)--;
}

void Todo_array_bubble_sort_priority(Todo todos[], int size)
{
  _Bool swapped;
  for (int i = 0; i < size-1; i++) {
    swapped = 0;
    for (int j = 0; j < size - i - 1; j++){
      if (todos[j+1].priority == 0)
        continue;
      if (todos[j].priority == 0) {
        Todo_swap(&todos[j], &todos[j+1]);
        swapped = 1;
        continue;
      }
      if (todos[j].priority > todos[j+1].priority) {
        Todo_swap(&todos[j], &todos[j+1]);
        swapped = 1;
      }
    }
    if (!swapped)
      break;
  }
}


void Todo_swap(Todo *t1, Todo *t2)
{
  Todo temp_t = *t1;
  *t1 = *t2;
  *t2 = temp_t;
}

unsigned int Todo_array_read_from_file(Todo todos[])
{
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  unsigned int output = 0;

  fp = fopen(Todo_file_path(),"r");
  if (fp == NULL)
    return output;
  char * note = malloc(4096*sizeof(char));
  char * todo = malloc(4096*sizeof(char));
  int index = 0;
  while ((read = getline(&line, &len, fp)) != -1) {
    char is_enabled;
    int scan_count = sscanf(line,"[%c]>%s %[^\n]",&is_enabled, note, todo);
    if (!strlen(todo)) {
      sscanf(line,"[%c] %[^\n]",&is_enabled, todo);
      strcpy(note, "");
    }
    if (is_enabled != '-') {
      int priority = is_enabled - '0';
      todos[output].done = FALSE;
      todos[output].priority = priority;
      strcpy(todos[output].message, todo);
      strcpy(todos[output].note, note);
      todos[output].file_index = index;
      output++;
    }
    strcpy(note, "");
    strcpy(todo, "");
    index++;
  }
  free(note);
  free(todo);

  Todo_array_bubble_sort_priority(todos, output);

  return output;
}

void Todo_array_print_ncurses(Todo todos[], int size)
{
  for (int i = 0; i < size; i++) {
    int is_done_ch = todos[i].done ? 'x' : ' ';
    mvprintw(TODOS_START+i, 0,"[%c] [%d] %s\n", is_done_ch, todos[i].priority, todos[i].message);
  }
}

#define MAX_CHAR 1000
void Todo_array_write_to_file(Todo todos[], int todos_size)
{
  FILE * fp_src;
  char line[MAX_CHAR];
  fp_src = fopen(Todo_file_path(),"r");
  #define TMP_FILE "/tmp/potato-c-todos"

  FILE * fp_tmp = fopen(TMP_FILE, "w");
  if (fp_src == NULL)
    return;
  if (fp_tmp == NULL)
    return;
  fseek(fp_src, 0, SEEK_SET);
  int current_line = 0;

  for (int i = 0; i < todos_size; i++) {
    if (todos[i].done) {
      fseek(fp_src, -strlen(line), SEEK_CUR);
      if (strlen(todos[i].note)) {
        fprintf(fp_tmp, "[-%d]>%s %s\n", todos[i].priority, todos[i].note, todos[i].message);
      } else {
        fprintf(fp_tmp, "[-%d] %s\n", todos[i].priority, todos[i].message);
      }
      continue;
    }
    if (strlen(todos[i].note)) {
      fprintf(fp_tmp, "[%d]>%s %s\n", todos[i].priority, todos[i].note, todos[i].message);
    } else {
      fprintf(fp_tmp, "[%d] %s\n", todos[i].priority, todos[i].message);
    }
  }

  while (fgets(line, sizeof(line), fp_src) != NULL) {
    char ch;
    sscanf(line, "[%c]", &ch);
    if (ch == '-') {
      fprintf(fp_tmp, "%s", line);
    } 
    current_line++;
  }
  fclose(fp_tmp);
  fclose(fp_src);
  if (remove(Todo_file_path()) != 0) {
    perror("Error deleting the source file");
    return;
  }

  if (rename(TMP_FILE, Todo_file_path()) != 0) {
    FILE *fp_new_src = fopen(Todo_file_path(), "w");
    fp_tmp = fopen(TMP_FILE, "r");
    while (fgets(line, sizeof(line), fp_tmp)) {
        fprintf(fp_new_src, "%s", line);
    }
    fclose(fp_new_src);
    fclose(fp_tmp);
    remove(TMP_FILE);
    return;
  }

}

void ncurses_clear_todos(int todos_size)
{
  for (int i = TODOS_START; i < TODOS_START+todos_size; i++) {
    move(i, 0);
    clrtoeol();
  }
}

void Todo_initialize(Todo *todo)
{
  todo->priority = -1;
  todo->file_index = -1;
  todo->done = 0;
}
