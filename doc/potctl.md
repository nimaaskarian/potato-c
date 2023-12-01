% POTCTL(1)
% Nima Askarian
% potctl-VERSION
# NAME
potctl - potato-c control

# SYNOPSIS
potctl [-cputqsidIDrT1 *index*(no index means all)] [-l]

# DESCRIPTION
potctl is the commandline controller of potato-c.
It can control any daemon running in your system in every way you may like.
It can be also used to output stuff about the daemon. Read more in *OPTIONS*
*index* mentioned in the *SYNOPSIS* is optional. If no instance is given, the command will be run on all the daemons.
If run with no options, it will do the same thing as using **-l**.

# OPTIONS

**-c *index***
: remove the pid file of a timer instance. Afterwards, the daemon can not be controlled with this or the TUI application. You may want to use this when a daemon has been quitted abruptly.

**-p *index*** 
: pause the daemon.

**-u *index***
: unpause the daemon.

**-t *index***
: toggle pause the daemon.

**-q *index***
: quit (kill) the daemon.

**-s *index***
: skip one mode to the next.

**-i *index***
: increase time left by 10 seconds.

**-d *index***
: decrease time left by 10 seconds.

**-I *index***
: increase pomodoro count by 1.

**-D *index***
: decrease pomodoro count by 1.

**-r *index***
: reset timer to the starting condition.

**-T *index***
: print the output of the daemon every second, just like the daemon itself would.

**-1 *index***
: print the output of the daemon only one time.

**-l**
: lists all the available daemons. Same effect as running without any arguments.

