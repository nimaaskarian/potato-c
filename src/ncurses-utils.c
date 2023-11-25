#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/ncurses-utils.h"
#include "../include/utils.h"

void ncurses_initialize_screen()
{
  initscr();
  if (has_colors()) {
    // alacritty no opaque background
    use_default_colors();
    start_color();
    init_pair(1,COLOR_BLACK, COLOR_WHITE);
  }
  // fast key :D
  cbreak();
  ncurses_unready_for_input();
}

void ncurses_change_color_line(int line, int color_pair)
{
  move(line, 0);
  chgat(-1, A_NORMAL, color_pair, NULL);
}

void ncurses_quit()
{
  endwin();
  exit(EXIT_SUCCESS);
}

void ncurses_clear_lines_from_to(int from, int to)
{
  for (int i = from; i < to; i++) {
    ncurses_clear_line(i);
  }
}

void ncurses_clear_line(int y)
{
  move(y, 0);
  clrtoeol();
}

void ncurses_ready_for_input()
{
  // blocking getch/getstr
  nodelay(stdscr, FALSE);
  // keypad(stdscr, TRUE);
  curs_set(1);
}

void str_n_at_i_appch(char * src, int n,int app_index, char ch)
{
  app_index = min(n, app_index);
  char *tmp = malloc(sizeof(char)*(n+1));
  for (int i = 0; i < app_index; i++) {
    tmp[i] = src[i];
  }
  tmp[app_index] = ch;
  for (int i = app_index; i < n; i++) {
    tmp[i+1] = src[i];
  }
  strncpy(src, tmp, n);
  src[n] = '\0';
  free(tmp);
}

int prev_space(char *str, int current_index)
{
  int space_index = 0;
  _Bool hit_non_space = FALSE;
  for (int i = current_index-1; i > -1; i--) {
    if (str[i] == ' ' && hit_non_space) {
      space_index = i+1;
      break;
    } 
    if (str[i] != ' ')  {
      hit_non_space = TRUE;
    }
  }
  return space_index;
}

int next_word(char *str, int n, int current_index)
{
  int word_start = n-1;
  _Bool hit_space = FALSE;
  for (int i = current_index+1; i < n; i++) {
    if (str[i] != ' ' && hit_space) {
      word_start = i;
      break;
    } 
    if (str[i] == ' ')  {
      hit_space = TRUE;
    }
  }
  return word_start;
}

int end_word(char *str, int n, int current_index)
{
  int word_end = n-1;
  _Bool hit_non_space = FALSE;
  for (int i = current_index+1; i < n; i++) {
    if (str[i] == ' ' && hit_non_space) {
      word_end = i-1;
      break;
    } 
    if (str[i] != ' ')  {
      hit_non_space = TRUE;
    }
  }
  return word_end;
}

typedef enum {
  INSERT = 0,
  NORMAL,
} Mode;

void update_string(char * str, int size, int start_y, int start_x)
{
  move(start_y,start_x);
  clrtoeol();
  refresh();
  printw("%.*s", size, str);
}

int str_remove_char_max(char * str, int size, int index, int max)
{
  if (index < max && size > 0) {
    memmove(&str[index], &str[index+1], size-index);
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}
typedef struct {
  int index;
  _Bool should_delete;
} Delete;
int ncurses_getnstr_default_vimode(char * src, const int max_size ,char * def)
{
  #define MODE_INDEX 4
  printw("(I): ");
  const unsigned int start_x = getcurx(stdscr);
  const unsigned int start_y = getcury(stdscr);
  int def_size = min(strlen(def), max_size), tmp_size = def_size;
  int index = tmp_size;
  printw("%.*s", def_size, def);
  char tmp[max_size+1];
  strncpy(tmp, def, def_size);
  tmp[max_size] = '\0';
  Mode mode = INSERT;
  Mode last_mode = mode;
  Delete delete = {.index=-1, .should_delete=FALSE};

  while (1) {
    if (delete.index != -1)
      delete.should_delete = TRUE;
    int ch = getch();
    if (ch == '\n') {
      strncpy(src,tmp, tmp_size);
      src[tmp_size] = '\0';
      return EXIT_SUCCESS;
    }
    // mvprintw(3, 0, "%d , %d", len, n);
    if (mode == INSERT) {
      switch(ch) {
        case KEY_LEFT:
          if (index > 0)
            index--;
          break;
        case KEY_RIGHT:
          if (index < tmp_size)
            index++;
          break;
        case ctrl(KEY_RIGHT): {
          index = prev_space(tmp, index);
        }
          break;
        case ctrl('u'):
          {
            tmp_size = tmp_size-index;
            memmove(tmp, &tmp[index], tmp_size);
            index=0;
            update_string(tmp, tmp_size, start_y, start_x);
          }
        case KEY_UP:
          index=0;
          break;
        case KEY_DOWN:
          index=tmp_size;
          break;
        case ctrl('w'):
          {
            int space_index = prev_space(tmp, index);
            tmp_size = tmp_size-(index-space_index);
            memmove(&tmp[space_index], &tmp[index], tmp_size);
            index=space_index;
            update_string(tmp, tmp_size, start_y, start_x);
          }
          break;
        case KEY_BACKSPACE:
        case KEY_DC:
        case 127:
          if (index > 0 && tmp_size > 0) {
            str_remove_char_max(tmp, tmp_size, index-1, max_size);
            tmp_size--;
            index--;
            // delch();
            // refresh();
            update_string(tmp, tmp_size, start_y, start_x);
          } 
          break;
        case ESC:
          if (index > 0)
            index--;
          curs_set(NORMAL);
          mode = NORMAL;
          break;
        default:
          if (tmp_size < max_size) {
            tmp_size++;
            str_n_at_i_appch(tmp, tmp_size, index, ch);
            update_string(tmp, tmp_size, start_y, start_x);
            index++;
          }
          break;
      }
    } else if (mode == NORMAL) {
    switch(ch) {
      case 'q':
      case ESC:
        // mvprintw(3,0,"%d", def_size);
        strncpy(src, def, def_size);
        src[def_size] = '\0';
        return EXIT_FAILURE;
      case 'l':
        if (index < tmp_size-1)
          index++;
        break;
      case '~':
        tmp[index] = toggle_lower(tmp[index]);
        update_string(tmp, tmp_size, start_y, start_x);
        if (index < tmp_size)
          index++;
      break;
      case 'd':
          delete.index = index;
        break;
      case 'x':
        if (str_remove_char_max(tmp, tmp_size, index, max_size) == EXIT_SUCCESS) {
          tmp_size--;
          update_string(tmp, tmp_size, start_y, start_x);
        }
      break;
      case 'h':
        if (index > 0)
          index--;
        break;
      case 'b':
        index = prev_space(tmp, index);
        break;
      case 'e':
        index = end_word(tmp, tmp_size, index);
      break;
      case 'w':
        index = next_word(tmp, tmp_size, index);
      break;
      case '^':
      case '0':
      case 'g':
        index = 0;
      break;
      case '$':
      case 'G':
        index = tmp_size-1;
      break;
      case 'A':
        index = tmp_size;
        mode=INSERT;
      break;
      case 'I':
        index = 0;
        mode=INSERT;
      break;
      case 'a':
        index++;
      case 'i':
        mode=INSERT;
      break;
    }
  }
    if (delete.index != -1 && delete.should_delete) {
      if (index == tmp_size-1){
        tmp[delete.index] = '\0';
        tmp_size = delete.index;
        index=delete.index-1;
      } else if (index > delete.index) {
        memmove(&tmp[delete.index], &tmp[index], tmp_size-index);
        tmp_size = tmp_size-(index-delete.index);
        index=delete.index;
      }  else if (index == delete.index) {
        if (str_remove_char_max(tmp,tmp_size,index,max_size) == EXIT_SUCCESS) {
          tmp_size--;
          index--;
        }
      } else {
        memmove(&tmp[index], &tmp[delete.index], tmp_size-delete.index);
        tmp_size = tmp_size-(delete.index-index);
      }
      update_string(tmp, tmp_size, start_y, start_x);
      delete.index = -1;
      delete.should_delete = FALSE;
    }
    if (last_mode != mode) {
      if (mode == INSERT)
        mvprintw(start_y, start_x-MODE_INDEX, "I");
      else
        mvprintw(start_y, start_x-MODE_INDEX, "N");
      last_mode = mode;
    }

    move(start_y, start_x+index);
    refresh();
    // printw("%d, %d", index, start_x);
  }
  return EXIT_SUCCESS;
}

void ncurses_unready_for_input()
{
  // non blocking getch/getstr
  nodelay(stdscr, TRUE);
  // keypad(stdscr, FALSE);
  curs_set(0);
  noecho();
}

typedef struct {
  int y, sleep;
} ClearLineThreadArgs;

static void * thread_function_clear_line(void * arguments)
{
  ClearLineThreadArgs *args = arguments;
  sleep(args->sleep);
  ncurses_clear_line(args->y);

  pthread_exit(EXIT_SUCCESS);
}

void ncurses_sleep_and_clear_line(int sleep, int y)
{
  ClearLineThreadArgs args;
  args.y = y;
  args.sleep = sleep;

  pthread_t clear_line_thread;
  pthread_create(&clear_line_thread, NULL, thread_function_clear_line, (void *)&args);
}

