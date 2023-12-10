#ifndef PIDFILE_H__
#define PIDFILE_H__
#include <linux/limits.h>

#define POTATO_PIDS_DIRECTORY "/tmp/potato-c"

char * get_pid_file_path(int pid);
void create_pid_file(int pid);
void remove_pid_file(int pid);
void remove_pid_file_by_port(int port);
#endif // PIDFILE_H__
