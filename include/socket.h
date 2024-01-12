#ifndef SOCKET_H__
#define SOCKET_H__
typedef enum {
  REQ_SECONDS = 0,
  REQ_TYPE,
  REQ_POMODOROS,
  REQ_PAUSED,
  REQ_TIMER_FULL,
} SocketRequest;
#define MAX_PORT 65535
#define MAX_PORT_LENGTH 5
#define NO_PORT -1
#define PORT_START 7700
#define PORT_STEP 1

void write_sock_port_to_pid_file(int pid, int sock_port);
int read_sock_port_from_pid_file(int pid);
int next_available_sock_port();
#endif // !SOCKET_H__
