#ifndef CLIENT_H__
#define CLIENT_H__
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../include/signal.h"
#include "../include/socket.h"
#include "../include/timer.h"

#define handle_kill_declare(NAME, SIG) void handle_##NAME(char *str, int index);

#define EVERY_MEMBER -1

handle_kill_declare(pause, SIG_PAUSE);
handle_kill_declare(quit, SIGQUIT);
handle_kill_declare(unpause, SIG_UNPAUSE);
handle_kill_declare(skip, SIG_SKIP);
handle_kill_declare(toggle_pause, SIG_TPAUSE);
handle_kill_declare(increase_10sec, SIG_INC_10SEC);
handle_kill_declare(decrease_10sec, SIG_DEC_10SEC);
handle_kill_declare(increase_pomodoro_count, SIG_INC_POMODORO_COUNT);
handle_kill_declare(decrease_pomodoro_count, SIG_DEC_POMODORO_COUNT);
handle_kill_declare(reset_pomodoro, SIG_RESET);

size_t run_function_on_pid_file_index(void(* handler)(char *, int index), int selected_index);
void run_function_on_pid_file_pid(void(* handler)(char *, int index), int selected_pid);
int connect_socket(int port, char * server_address);
int send_socket_request_return_num(SocketRequest req, int pid, char * server_address);
unsigned int get_pids_length();
pid_t pid_at_index(unsigned int selected_index);
Timer * request_timer(pid_t pid);
void handle_remove_pid(char *name, int index);
int send_socket_request_with_fd(SocketRequest req, int sockfd);
Timer get_timer_from_port(int port, char * server_address);
Timer get_local_timer_from_pid(int pid);
#endif // !CLIENT_H__
