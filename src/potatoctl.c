#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>

#include "lib/signal.h"

#define EVERY_MEMBER -1
void run_on_dir_members(void(* handler)(char *, int index), int selected_index)
{
  DIR *dp;
  struct dirent *ep;
  dp = opendir (POTATO_PIDS_DIRECTORY);
  int index = 0;

  if (dp != NULL)
  {
    while ((ep = readdir (dp)) != NULL) {
      if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")) {
        if (index == selected_index || selected_index == EVERY_MEMBER)
          handler(ep->d_name, index);
        index++;
      }
    }

    (void) closedir (dp);
    return;
  }
}
void handle_dir_member_list(char * str, int index)
{
  printf("%d\t%s\n", index, str);
}

#define handle_kill(NAME, SIG) void handle_##NAME(char *str, int index)\
{\
  kill(atoi(str), SIG);\
}

#define case_index(ch,function) case ch:\
{\
  int index = EVERY_MEMBER;\
  if (optarg != NULL)\
    sscanf(optarg, "%d", &index);\
  run_on_dir_members(function, index);\
  break;\
}

handle_kill(pause, SIG_PAUSE);
handle_kill(quit, SIGQUIT);
handle_kill(unpause, SIG_UNPAUSE);
handle_kill(skip, SIG_SKIP);
handle_kill(toggle_pause, SIG_TPAUSE);
handle_kill(increase_10sec, SIG_INC_10SEC);
handle_kill(decrease_10sec, SIG_DEC_10SEC);
handle_kill(increase_pomodoro_count, SIG_DEC_POMODORO_COUNT);
handle_kill(decrease_pomodoro_count, SIG_DEC_POMODORO_COUNT);

void remove_potato_pid_file(char *name, int index)
{
  char path[PATH_MAX];
  snprintf(path, PATH_MAX, "%s/%s", POTATO_PIDS_DIRECTORY, name);

  remove(path);
}
int main(int argc, char *argv[])
{   
  int ch;
  while ((ch = getopt(argc, argv, "clu::L::s::p::t::q::d::i::I::D::")) != -1) {
    switch (ch) {
      case 'l': 
        run_on_dir_members(handle_dir_member_list, EVERY_MEMBER);
      break;

      case 'c': 
        run_on_dir_members(remove_potato_pid_file, EVERY_MEMBER);
      break;

      case_index('p', handle_pause);
      case_index('u', handle_unpause);
      case_index('t', handle_toggle_pause);
      case_index('q', handle_quit);
      case_index('s', handle_skip);
      case_index('i', handle_increase_10sec);
      case_index('d', handle_decrease_10sec);
      case_index('I', handle_decrease_pomodoro_count);
      case_index('D', handle_increase_pomodoro_count);
    }
  }
  return EXIT_SUCCESS;
}
