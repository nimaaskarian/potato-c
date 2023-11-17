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
  keypad(stdscr, TRUE);
  curs_set(1);
  echo();
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

void ncurses_getn_default_str(char * src ,char * def, const int n)
{
  const unsigned int start_x = getcurx(stdscr);
  const unsigned int start_y = getcury(stdscr);
  int def_len = min(strlen(def), n), len = def_len;
  int index = len;
  printw("%.*s", def_len, def);
  char tmp[n];
  strncpy(tmp, def, def_len);
  tmp[n-1] = '\0';

  while (1) {
    int ch = getch();
    // mvprintw(3, 0, "%d , %d", len, n);
    switch(ch) {
      case KEY_LEFT:
        if (index > 0)
          index--;
      break;
      case KEY_RIGHT:
        if (index < len)
          index++;
      break;
      case ctrl('u'):
       {
          len = len-index;
          memmove(tmp, &tmp[index], len);
          index=0;
          move(start_y,start_x);
          clrtoeol();
          refresh();
          printw("%.*s", len, tmp);
        }
      case KEY_UP:
        index=0;
      break;
      case KEY_DOWN:
        index=len;
      break;
      case ctrl('w'):
       {
          int space_index = 0;
          _Bool hit_non_space = FALSE;
          for (int i = index-1; i > -1; i--) {
            if (tmp[i] == ' ' && hit_non_space) {
              space_index = i+1;
              break;
            } 
            if (tmp[i] != ' ')  {
              hit_non_space = TRUE;
            }
          }
          len = len-(index-space_index);
          memmove(&tmp[space_index], &tmp[index], len);
          index=space_index;
          move(start_y,start_x);
          clrtoeol();
          refresh();
          printw("%.*s", len, tmp);
        }
      break;
      case KEY_BACKSPACE:
        if (index > 0 && len > 0) {
          if (index < n)
            memmove(&tmp[index], &tmp[index+1], len-index);
          len--;
          index--;
          delch();
          refresh();
        } 
        break;
      case ESC:
        strncpy(src, def, def_len);
        return;
      break;
      case '\n':
        strcpy(src,tmp);
        src[len] = '\0';
      return;
      default:
        if (len < n-1) {
          len++;
          str_n_at_i_appch(tmp, len, index, ch);
          // mvprintw(3,0,"%s",tmp);
          move(start_y, start_x);
          clrtoeol();
          printw("%s",tmp);
          index++;
        }
      break;
    }
    move(start_y, start_x+index);
    refresh();
    // printw("%d, %d", index, start_x);
  }

}

void ncurses_unready_for_input()
{
  // non blocking getch/getstr
  nodelay(stdscr, TRUE);
  keypad(stdscr, FALSE);
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

