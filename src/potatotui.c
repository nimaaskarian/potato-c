#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>

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
  mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
  printf("\033[?1003h\n");
  /* User input imediatly avaiable */
  mouseinterval(0);
  raw();
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
    case 'd':
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

void *timer_thread(void *arg) {
  Timer *timer = NULL;
  while (1) {
    if (pid)
      timer = request_timer(pid);

    if (timer == NULL)
      timer_thread_paused = 1;

    if (!timer_thread_paused) {
      move(0,0);
      clrtoeol();
      printw("Time left: %02d:%02d\nPomodoros: %d\nType: %s\n",timer->seconds/60, timer->seconds%60, timer->pomodoro_count, type_string(timer->type));
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
    case 'd':
    case 'D':
      run_function_on_pid_file_pid(handle_quit, pid);
    break;
  }
}

int main(int argc, char *argv[])
{
  initialize_screen();
  int selected_index = 0;
  pthread_t thread1;

  while (1) {
    timer_thread_paused = 1;
    pid = 0;
    pid = pid_selection_menu(&selected_index);
    timer_thread_paused = 0;

    erase();
    pthread_create(&thread1, NULL, timer_thread, NULL);
    while(timer_thread_paused == 0) {
      int ch = getch();
      if (handle_input_character_common(ch))
        break;
      handle_input_timer(ch);
      napms(1000 / 60);
    }
  }
  return EXIT_SUCCESS;
}
