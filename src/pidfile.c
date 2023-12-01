#include <linux/limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/pidfile.h"

void get_pid_file_path(int pid, char pid_path[PATH_MAX])
{
  snprintf(pid_path, PATH_MAX, "%s/%d", POTATO_PIDS_DIRECTORY, pid);
}
void create_pid_file(int pid)
{
  struct stat st = {0};
  if (stat(POTATO_PIDS_DIRECTORY, &st) == -1) {
    mkdir(POTATO_PIDS_DIRECTORY, 0700);
  }

  char pid_path[PATH_MAX];
  get_pid_file_path(pid, pid_path);

  FILE* file_ptr = fopen(pid_path, "w");
  fclose(file_ptr);
}

void remove_pid_file(int pid)
{
  char pid_path[PATH_MAX];
  get_pid_file_path(pid, pid_path);

  remove(pid_path);
}

