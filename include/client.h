#ifndef CLIENT_H__
#define CLIENT_H__
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../include/signal.h"
#include "../include/socket.h"
#include "../include/timer.h"

#define handle_kill(NAME, SIG) void handle_##NAME(char *str, int index);

#define EVERY_MEMBER -1

handle_kill(pause, SIG_PAUSE);
handle_kill(quit, SIGQUIT);
handle_kill(unpause, SIG_UNPAUSE);
handle_kill(skip, SIG_SKIP);
handle_kill(toggle_pause, SIG_TPAUSE);
handle_kill(increase_10sec, SIG_INC_10SEC);
handle_kill(decrease_10sec, SIG_DEC_10SEC);
handle_kill(increase_pomodoro_count, SIG_INC_POMODORO_COUNT);
handle_kill(decrease_pomodoro_count, SIG_DEC_POMODORO_COUNT);
handle_kill(reset_pomodoro, SIG_RESET);

void run_function_on_pid_file_index(void(* handler)(char *, int index), int selected_index);
void run_function_on_pid_file_pid(void(* handler)(char *, int index), int selected_pid);
int connect_socket(int port);
int send_socket_request_return_num(SocketRequest req, int pid);
unsigned int get_pids_length();
pid_t pid_at_index(unsigned int selected_index);
Timer * request_timer(pid_t pid);
void handle_remove_pid(char *name, int index);
int send_socket_request_with_fd(SocketRequest req, int sockfd);
Timer get_timer_pid(pid_t pid);
int is_socket_available(int port);
#endif // !CLIENT_H__
