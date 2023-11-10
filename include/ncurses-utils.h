#ifndef __NCURSES_UTILS_H
#define __NCURSES_UTILS_H

void change_color_line(int line, int color_pair);
void initialize_screen();
void ncurses_quit();
void clear_lines_from_to(int from, int to);
void clear_line(int y);
void enable_delay_and_echo();
void disable_delay_and_echo();
#define MAX_Y getmaxy(stdscr)-1

#endif // __NCURSES_UTILS_H
