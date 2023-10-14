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

#define handle_kill_member_list(NAME, SIG) void handle_##NAME##_member_list(char *str, int index)\
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

handle_kill_member_list(pause, SIG_PAUSE);
handle_kill_member_list(quit, SIGQUIT);
handle_kill_member_list(unpause, SIG_UNPAUSE);
handle_kill_member_list(skip, SIG_SKIP);
handle_kill_member_list(toggle_pause, SIG_TPAUSE);


int main(int argc, char *argv[])
{   
  int ch;
  char **processes;
  while ((ch = getopt(argc, argv, "lu::s::p::t::")) != -1) {
    switch (ch) {
      case 'l': 
        run_on_dir_members(handle_dir_member_list, EVERY_MEMBER);
      break;
      case_index('p', handle_pause_member_list);
      case_index('u', handle_unpause_member_list);
      case_index('t', handle_toggle_pause_member_list);
      case_index('s', handle_skip_member_list);
    }
  }
  return EXIT_SUCCESS;
}
