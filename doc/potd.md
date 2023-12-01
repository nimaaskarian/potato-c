% POTD(1)
% Nima Askarian
% potd-VERSION
# NAME
potd - potato-c daemon

# SYNOPSIS
potd [-fnpNs]

# DESCRIPTION
potd is the daemon application of potato-c. This is the core. The two other binaries just control and connect to these application. The time left of your timer, counts of your pomodoros, and some formatting strings specifed in config.h at the time of compilation are printed in the stdout. This makes potd to even get called directly inside your bar. Of course, this would restart your timer every time you restart your bar.

# OPTIONS

**-f**
: flushes the output after every write. Suitable for using directly in bars.

**-n**
: send notifications after each event. This option needs libnotify (notify-send) to be installed.

**-p**
: print the count of pomodoro, not just the timer.

**-N**
: print a new line at quit. Suitable for using directly in bars

**-s**
: don't run socket server. The default behavior runs a socket server.
