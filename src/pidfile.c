#include <linux/limits.h>
#include <string.h>
#include <dirent.h>
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
#include "../include/socket.h"

char * get_pid_file_path(int pid)
{
  char * output;
  asprintf(&output, "%s/%d", POTATO_PIDS_DIRECTORY, pid);
  return output;
}
void create_pid_file(int pid)
{
  struct stat st = {0};
  if (stat(POTATO_PIDS_DIRECTORY, &st) == -1) {
    mkdir(POTATO_PIDS_DIRECTORY, 0700);
  }

  char *pid_path = get_pid_file_path(pid);

  FILE* file_ptr = fopen(pid_path, "w");
  free(pid_path);
  fclose(file_ptr);
}

void remove_pid_file(int pid)
{
  char*pid_path = get_pid_file_path(pid);

  remove(pid_path);
  free(pid_path);
}

void remove_pid_file_by_port(int port)
{
  DIR *dp;
  struct dirent *ep;
  dp = opendir (POTATO_PIDS_DIRECTORY);
  char * port_str;
  asprintf(&port_str, "%d", port);
  int index = 0;

  if (dp != NULL)
  {
    while ((ep = readdir (dp)) != NULL) {
      if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")) {
        char content[MAX_PORT_LENGTH];
        char * path;
        asprintf(&path,"%s/%s", POTATO_PIDS_DIRECTORY, ep->d_name);
        FILE *fp = fopen(path, "r");
        fgets(content,MAX_PORT_LENGTH,fp);
        if (strcmp(port_str, content) == 0)
          remove(path);
        free(path);
        fclose(fp);
      }
    }

    (void) closedir (dp);
  }
  free(port_str);
  free(ep);
}


