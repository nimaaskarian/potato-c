#include <stdio.h>
#include <linux/limits.h>
#include <stdlib.h>
#include "../include/socket.h"
#include "../include/client.h"
#include "../include/pidfile.h"

int is_socket_available(int port)
{
  int sockfd;
  struct sockaddr_in serverAddr;

  // Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Set server address details
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Replace with desired IP address

  // Connect to the server
  int connectionStatus = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
  if (connectionStatus == 0) {
    close(sockfd);
    return 1; // Socket connection exists
  }

  close(sockfd);
  return 0; // Socket connection does not exist
}

void write_sock_port_to_pid_file(int pid, int sock_port)
{
  if (sock_port == NO_PORT)
    return;
  FILE *fptr;
  char *path = get_pid_file_path(pid);

  fptr = fopen(path, "w");
  free(path);
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
  char *path = get_pid_file_path(pid);

  fptr = fopen(path, "r");
  free(path);
  if (!fptr)
    return NO_PORT;
  int sock_port;
  
  int scanf_status = fscanf(fptr, "%d", &sock_port);
  fclose(fptr);
  if (scanf_status == 1)
    return sock_port;

  return NO_PORT;
}
