#include <ncurses.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/ncurses-utils.h"

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
  curs_set(1);
  echo();
}

void ncurses_unready_for_input()
{
  // non blocking getch/getstr
  nodelay(stdscr, TRUE);
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

