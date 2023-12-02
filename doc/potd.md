% POTD(1)
% Nima Askarian
% potd-VERSION
# NAME
potd - potato-c daemon

# SYNOPSIS
potd [-fnpNs] [+*FORMAT*{%b%t%B%p}]

# DESCRIPTION
potd is the daemon application of potato-c. This is the core. The two other binaries just control and connect to these application. The time left of your timer, counts of your pomodoros, and some formatting strings specifed in config.h at the time of compilation are printed in the stdout. This makes potd to even get called directly inside your bar. Of course, this would restart your timer every time you restart your bar.

# OPTIONS

**-f**
: flushes the output after every write. Suitable for using directly in bars.

**-n**
: send notifications after each event. This option needs libnotify (notify-send) to be installed.

**+*FORMAT***
: format string of how the daemon prints the timer. See more about it in *FORMAT* section

**-N**
: print a new line at quit. Suitable for using directly in bars

**-s**
: don't run socket server. The default behavior runs a socket server.

# FORMAT
Format can be any string, only the string below has meaning for our timer.

**%b**
: before_timer string (which indicates the mode of timer, can be changed in config.h at compile time).

**%t**
: time remaining of the timer in hours:minutes:seconds or minutes:seconds.

**%B**
: before pomodoro timer. This is a static string. Can be changed in config.h at compile time.

**%p**
: count of pomodoros left.

**%f**
: flush the output. Suitable for using in bars, at the end of the format string, without the **-f** option.
