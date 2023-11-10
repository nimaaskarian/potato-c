#include <ncurses.h>
#include <stdlib.h>

#include "../include/ncurses-utils.h"

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
  // raw();
  cbreak();
  /* Invisible cursor */
  curs_set(0);
  /* Non-blocking getch */
  nodelay(stdscr, TRUE);
  /* Enable keypad */
  // keypad(stdscr, TRUE);

}

void change_color_line(int line, int color_pair)
{
  move(line, 0);
  chgat(-1, A_NORMAL, color_pair, NULL);
}


void ncurses_quit()
{
  endwin();
  exit(EXIT_SUCCESS);
}

void clear_lines_from_to(int from, int to)
{
  for (int i = from; i < to; i++) {
    clear_line(i);
  }
}

void clear_line(int y)
{
  move(y, 0);
  clrtoeol();
}

void enable_delay_and_echo()
{
  nodelay(stdscr, FALSE);
  curs_set(1);
  echo();
}

void disable_delay_and_echo()
{
  nodelay(stdscr, TRUE);
  curs_set(0);
  noecho();
}
