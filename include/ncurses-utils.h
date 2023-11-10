#ifndef __NCURSES_UTILS_H
#define __NCURSES_UTILS_H

void ncurses_change_color_line(int line, int color_pair);
void ncurses_initialize_screen();
void ncurses_quit();
void ncurses_clear_lines_from_to(int from, int to);
void ncurses_clear_line(int y);
void ncurses_ready_for_input();
void ncurses_unready_for_input();
#define MAX_Y getmaxy(stdscr)-1

#endif // __NCURSES_UTILS_H
