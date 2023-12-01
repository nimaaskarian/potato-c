#ifndef PIDFILE_H__
#define PIDFILE_H__
#include <linux/limits.h>

#define POTATO_PIDS_DIRECTORY "/tmp/potato-c"

void get_pid_file_path(int pid, char output[PATH_MAX]);
void create_pid_file(int pid);
void remove_pid_file(int pid);
#endif // PIDFILE_H__
