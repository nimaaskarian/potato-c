#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include <pthread.h>
#include "../include/timer.h"

Timer timer;

void initialize_screen()
{
  initscr();
  if (has_colors()) {
    use_default_colors();
    start_color();
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

void handle_input_character(int ch)
{
  switch (ch) {
    case 'q':
    case 'Q': 
      quit();
    break;
  }
}

void *timer_thread(void *arg) {
  while (1) {
    erase();
    printw("%02d:%02d\n",timer.seconds/60, timer.seconds%60);
    Timer_reduce_second_sleep(&timer);
  }
	pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
  initialize_screen();
  Timer_initialize(&timer);
  Timer_set_seconds_based_on_type(&timer);
  pthread_t thread1;
  pthread_create(&thread1, NULL, timer_thread, NULL);

  while(1) {
    handle_input_character(getch());
  }
  return EXIT_SUCCESS;
}
// int main() 
// {
//   Timer timer;
//   initialize_screen();
//   fd_set set;
//   struct timeval tv;
//   int ret;

//   // Set up the file descriptor set to wait for input from stdin
//   FD_ZERO(&set);
//   FD_SET(STDIN_FILENO, &set);

//   // Set up the timeout to sleep for 1 second
//   tv.tv_sec = 1;
//   tv.tv_usec = 0;

//   // Loop until input is received from stdin
//   while (1) {
//     // Wait for input from stdin with no delay
//     ret = select(STDIN_FILENO + 1, &set, NULL, NULL, &tv);

//     printw("%d", ret);
//     if (ret == -1) {
//       perror("select");
//       exit(EXIT_FAILURE);
//     } else if (ret == 1) {
//       handle_input_character(getchar());
//     } else {
//       erase();
//       Timer_reduce_second_sleep(&timer);
//     }
//     char * time_left = Timer_time_left(&timer);
//     printw("%s\n",time_left);
//     free(time_left);
//   }

//   return 0;
// }
