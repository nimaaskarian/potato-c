#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../include/signal.h"
#include "../include/socket.h"
#include "../include/utils.h"
#include "../include/client.h"

#define case_index(ch,function) case ch:\
{\
  int index = EVERY_MEMBER;\
  if (optarg != NULL)\
    sscanf(optarg, "%d", &index);\
  run_function_on_pid_file_index(function, index);\
  break;\
}

void remove_potato_pid_file(char *name, int index)
{
  char path[PATH_MAX];
  snprintf(path, PATH_MAX, "%s/%s", POTATO_PIDS_DIRECTORY, name);

  remove(path);
}

int connect_socket(int PORT)
{
    int status, valread, client_fd;
    struct sockaddr_in serv_addr;
    char buffer[1024] = { 0 };
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
 
    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)
        <= 0) {
        printf(
            "\nInvalid address/ Address not supported \n");
        return -1;
    }
 
    if ((status
         = connect(client_fd, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr)))
        < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
  
  return client_fd;
}

void send_socket_request(SocketRequest req, int port, char buffer[])
{
  size_t size = int_length(req)+1;
  char* request = malloc(size*sizeof(char));
  snprintf(request, size, "%d",req);
  
  int sockfd = connect_socket(port);
  send(sockfd, request, size, 0);

  int valread = read(sockfd, buffer, 1024 - 1);
  close(sockfd);
}

void get_type(char *str, int index)
{
  char buffer[1024];
  send_socket_request(REQ_TYPE,atoi(str),buffer);

  puts(buffer);
}

void get_seconds(char *str, int index)
{
  char buffer[1024];
  send_socket_request(REQ_SECONDS,atoi(str),buffer);
  
  puts(buffer);
}

int main(int argc, char *argv[])
{   
  int ch;
  while ((ch = getopt(argc, argv, "T::S::clu::L::s::p::t::q::d::i::I::D::r::")) != -1) {
    switch (ch) {
      case 'l': 
        run_function_on_pid_file_index(handle_list_pid_files, EVERY_MEMBER);
      break;

      case 'c': 
        run_function_on_pid_file_index(remove_potato_pid_file, EVERY_MEMBER);
      break;

      case_index('p', handle_pause);
      case_index('u', handle_unpause);
      case_index('t', handle_toggle_pause);
      case_index('q', handle_quit);
      case_index('s', handle_skip);
      case_index('i', handle_increase_10sec);
      case_index('d', handle_decrease_10sec);
      case_index('I', handle_increase_pomodoro_count);
      case_index('D', handle_decrease_pomodoro_count);
      case_index('r', handle_reset_pomodoro);
      case_index('T', get_type);
      case_index('S', get_seconds);
    }
  }
  return EXIT_SUCCESS;
}
