#ifndef CLIENT_H__
#define CLIENT_H__
#include "../include/signal.h"
#include "../include/socket.h"

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
void handle_list_pid_files(char * str, int index);
#endif // !CLIENT_H__
