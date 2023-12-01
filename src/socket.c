#include <stdio.h>
#include <linux/limits.h>
#include <stdlib.h>
#include "../include/socket.h"
#include "../include/client.h"
#include "../include/pidfile.h"

void write_sock_port_to_pid_file(int pid, int sock_port)
{
  if (sock_port == NO_PORT)
    return;
  FILE *fptr;
  char path[PATH_MAX];
  get_pid_file_path(pid, path);

  fptr = fopen(path, "w");
  fprintf(fptr, "%d", sock_port);
  fclose(fptr);
}

int next_available_sock_port()
{
  int port = PORT_START;
  int sockfd;
  while (is_socket_available(port)) {
    port+=PORT_STEP;
  }
  if (port > MAX_PORT)
    return NO_PORT;
  close(sockfd);

  return port;

}

int read_sock_port_from_pid_file(int pid)
{
  FILE *fptr;
  char path[PATH_MAX];
  get_pid_file_path(pid, path);

  fptr = fopen(path, "r");
  // if (!fptr)
  //   return NO_PORT;
  int sock_port;
  
  int scanf_status = fscanf(fptr, "%d", &sock_port);
  fclose(fptr);
  if (scanf_status == 1)
    return sock_port;

  return NO_PORT;
}
