#ifndef __NCURSES_UTILS_H
#define __NCURSES_UTILS_H
#define ctrl(x)           ((x) & 0x1f)
#define ESC 27
void ncurses_change_color_line(int line, int color_pair);
void ncurses_initialize_screen();
void ncurses_quit();
void ncurses_clear_lines_from_to(int from, int to);
void ncurses_clear_line(int y);
void ncurses_ready_for_input();
void ncurses_unready_for_input();
void ncurses_sleep_and_clear_line(int sleep, int y);
int ncurses_getnstr_default_vimode(char * src, const int tmp_max ,char * def);
#define MAX_Y getmaxy(stdscr)-1

#endif // __NCURSES_UTILS_H
