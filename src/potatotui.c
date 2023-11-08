#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include <ncurses.h>

#include "../include/timer.h"
#include "../include/client.h"

void initialize_screen()
{
  initscr();
  if (has_colors()) {
    use_default_colors();
    start_color();
    init_pair(1,COLOR_BLACK, COLOR_WHITE);
  }
  /* User input dont appear at screen */
  noecho();
  /* Makes terminal report mouse movement events */
  // mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  // printf("\033[?1003h\n");
  /* User input imediatly avaiable */
  // mouseinterval(0);
  raw();
  cbreak();
  /* Invisible cursor */
  curs_set(0);
  /* Non-blocking getch */
  nodelay(stdscr, TRUE);
  /* Enable keypad */
  keypad(stdscr, TRUE);

}

void quit()
{
  endwin();
  exit(EXIT_SUCCESS);
}

#define ESC 27
int handle_input_character_common(int ch)
{
  switch (ch) {
    case 'q':
    case 'Q': 
      quit();
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

void check_pids_length_and_set_selected(int *selected_index)
{
  if (*selected_index < 0) {
    *selected_index = pids_length()-1;
    return;
  }

  if (*selected_index >= pids_length())  {
    *selected_index = 0;
  }
}

void handle_input_pid_menu(int ch, int *selected_index, pid_t * pid)
{
  switch (ch) {
    case 'j':
    case 'J':
      (*selected_index)++;
      check_pids_length_and_set_selected(selected_index);
      break;
    case 'k':
    case 'K':
      (*selected_index)--;
      check_pids_length_and_set_selected(selected_index);
      break;
    case 'g':
      *selected_index = 0;
      break;
    case 'C':
    case 'c':
      run_function_on_pid_file_index(remove_potato_pid_file, *selected_index);
      break;
    case 'D':
    // case 'd':
      run_function_on_pid_file_index(handle_quit, *selected_index);
      break;
    case 'G':
      *selected_index = pids_length()-1;
      break;
    case '\n':
      *pid = pid_at_index(*selected_index);
    break;
  }
}

int pids_length_global;
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

void * pids_length_thread(void* args)
{
  while (1) {
    pids_length_global = pids_length();
    sleep(2);
  }
  pthread_exit(EXIT_SUCCESS);
}

pid_t pid_selection_menu(int *selected_index)
{
  pthread_t thread2;
  pthread_create(&thread2, NULL, pids_length_thread, NULL);

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
  return output;
}

pid_t pid;
_Bool timer_thread_paused = 0;

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

char * todos_path()
{
  const char* home = getenv("HOME");
  char *todo_path = malloc(4096*sizeof(char));
  snprintf(todo_path, 4096, "%s/.local/share/calcurse/todo", home);

  return todo_path;
}
typedef struct {
  int file_index, priority;
  char note[4096], message[4096];
  _Bool done;
} Todo;

unsigned int read_todos(Todo todos[])
{
  FILE * fp;
  char * line = NULL;
  size_t len = 0;
  ssize_t read;
  unsigned int output = 0;

  fp = fopen(todos_path(),"r");
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

  return output;
}

#define TODOS_START 5
void write_todos_to_scr(Todo todos[], int size)
{
  for (int i = 0; i < size; i++) {
    int is_done_ch = todos[i].done ? 'x' : ' ';
    mvprintw(TODOS_START+i, 0,"[%c] [%d] %s\n", is_done_ch, todos[i].priority, todos[i].message);
  }
}

void write_todos_to_file(Todo todos[], int todos_size);
void change_color_line(int line, int color_pair);
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
      char str[100];
      Todo todo;
      todo.priority = -1;
      timer_thread_paused = 1;
      nodelay(stdscr, FALSE);
      echo();

      while (todo.priority < 0 || todo.priority > 9){
        mvprintw(getmaxy(stdscr)-1, 0, "Todo priority [0]: ");
        getstr(str);
        todo.priority = atoi(str);
      }
      move(getmaxy(stdscr)-1, 0);
      clrtoeol();
      mvprintw(getmaxy(stdscr)-1, 0, "Todo message: ");
      int i = 0;
      getstr(str);
      strcpy(todo.message, str);

      noecho();
      timer_thread_paused = 0;
      nodelay(stdscr, TRUE);

      move(getmaxy(stdscr)-1, 0);
      clrtoeol();
      todo.done = 0;
      todo.file_index = -1;
      todos[*todos_size] = todo;
      (*todos_size)++;
      write_todos_to_scr(todos, *todos_size);
      change_color_line(TODOS_START+*selected_index, 1);
      break;
    }
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
    case 'w':
      write_todos_to_file(todos, *todos_size);
      for (int i = TODOS_START; i < TODOS_START+*todos_size; i++) {
        move(i, 0);
        clrtoeol();
      }
      *todos_size = read_todos(todos);
      write_todos_to_scr(todos, *todos_size);
      mvprintw(getmaxy(stdscr)-1, 0, "Wrote to file");
      while (*selected_index >= *todos_size && *selected_index > 0)
        (*selected_index)--;
      change_color_line(TODOS_START+*selected_index, 1);
    break;
  }
  if (*selected_index >= *todos_size)
    *selected_index = 0;
  if (*selected_index < 0) {
    *selected_index = *todos_size-1 < 0 ? 0 : *todos_size-1;
  }
}

void *timer_thread(void *arg) {
  Timer *timer = NULL;
  while (1) {
    if (pid)
      timer = request_timer(pid);

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
    // sleep(1);
    napms(1000 / 60);
  }
  free(timer);
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
      run_function_on_pid_file_pid(handle_decrease_10sec, pid);
    break;
    case '+':
    case 'K':
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

void change_color_line(int line, int color_pair)
{
  move(line, 0);
  chgat(-1, A_NORMAL, color_pair, NULL);
}

#define MAX_CHAR 1000
void write_todos_to_file(Todo todos[], int todos_size)
{
  if (todos_size < 1)
    return;
  FILE * fp_src;
  char line[MAX_CHAR];
  fp_src = fopen(todos_path(),"r");
  #define TMP_FILE "/tmp/potato-c-todos"

  FILE * fp_tmp = fopen(TMP_FILE, "w");
  if (fp_src == NULL)
    return;
  if (fp_tmp == NULL)
    return;
  fseek(fp_src, 0, SEEK_SET);
  int current_line = 0;


  int todos_index = 0;
  while (fgets(line, sizeof(line), fp_src) != NULL) {
    #define CURRENT_TODO todos[todos_index]
    if (todos_index >= todos_size)
      break;
    if (current_line != CURRENT_TODO.file_index || !CURRENT_TODO.done) {
      if (todos_index)
        fprintf(fp_tmp, "\n");
      fprintf(fp_tmp, "%s", line);
      todos_index++;
    }
    current_line++;
  }
  for (int i = 0; i < todos_size; i++) {
    #define CURRENT_TODO todos[i]
    if (CURRENT_TODO.done) {
      fseek(fp_src, -strlen(line), SEEK_CUR);
      if (strlen(CURRENT_TODO.note)) {
        fprintf(fp_tmp, "[-%d]>%s %s\n", CURRENT_TODO.priority, CURRENT_TODO.note, CURRENT_TODO.message);
      } else {
        fprintf(fp_tmp, "[-%d] %s\n", CURRENT_TODO.priority, CURRENT_TODO.message);
      }
      continue;
    }
    if (CURRENT_TODO.file_index == -1) {
      if (strlen(CURRENT_TODO.note)) {
        fprintf(fp_tmp, "[%d]>%s %s\n", CURRENT_TODO.priority, CURRENT_TODO.note, CURRENT_TODO.message);
      } else {
        fprintf(fp_tmp, "[%d] %s\n", CURRENT_TODO.priority, CURRENT_TODO.message);
      }
    }
  }
  fclose(fp_tmp);
  fclose(fp_src);
  if (remove(todos_path()) != 0) {
    perror("Error deleting the source file");
    return;
  }

  if (rename(TMP_FILE, todos_path()) != 0) {
    // perror("Error renaming temp file");
    // perror("Error renaming temp file");
    // If renaming fails, you can copy the content instead        
    FILE *fp_new_src = fopen(todos_path(), "w");
    fp_tmp = fopen(TMP_FILE, "r");
    while (fgets(line, sizeof(line), fp_tmp)) {
        fprintf(fp_new_src, "%s", line);
    }
    fclose(fp_new_src);
    fclose(fp_tmp);
    return;
  }

}


#define MAX_TODOS 40
void start_timer_loop_on_thread(pthread_t *thread)
{
  pthread_create(thread, NULL, timer_thread, NULL);
  mvprintw(TODOS_START-1, 0,"Todos:");
  Todo todos[MAX_TODOS];
  int todos_size = read_todos(todos);
  write_todos_to_scr(todos, todos_size);

  int selected_index = 0;
  change_color_line(TODOS_START+selected_index, 1);

  while(timer_thread_paused == 0) {
    int ch = getch();
    if (handle_input_character_common(ch))
      break;
    int prior_selected_index = selected_index;
    handle_input_todos_menu(ch, &selected_index, &todos_size, todos);

    handle_input_timer(ch);

    if (prior_selected_index != selected_index) {
      change_color_line(TODOS_START+selected_index, 1);
      change_color_line(TODOS_START+prior_selected_index, 0);
    }
    napms(1000 / 60);
  }
}

int main(int argc, char *argv[])
{
  pthread_t thread1;
  initialize_screen();
  int selected_index = 0;
  while (1) {
    timer_thread_paused = 1;
    pid = 0;
    pid = pid_selection_menu(&selected_index);
    timer_thread_paused = 0;

    start_timer_loop_on_thread(&thread1);
  }
  return EXIT_SUCCESS;
}
