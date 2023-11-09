#ifndef SOCKET_H__
#define SOCKET_H__
typedef enum {
  REQ_SECONDS = 0,
  REQ_TYPE,
  REQ_POMODOROS,
  REQ_PAUSED,
  REQ_TIMER_FULL,
} SocketRequest;
int return_sock_port_from_number(int n);
#define MAX_PORT 65535
#endif // !SOCKET_H__
