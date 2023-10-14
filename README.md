# Potato-c 
Potato-c is a no BS pomodoro timer for GNU/Linux operating systems.

## The Pomodoro method
This method is about breaking your tasks into small intervals so you don't get zoned out from not having enough tik tok videos.

## Why potato-c
This program's base idea is from [Tomato.C](https://github.com/gabrielzschmitz/Tomato.C), "a pure C pomodoro timer",
which after trying to fork it, the ideas behind it seemed problematic to me.  
This program tries to be able to run on a potato (as the name suggests)
- Blazingly Fast and efficient
- Is modular and simple
- It is based on POSIX Philosophy, does one thing and one thing only.

## Potatod
One of the binaries from this program is `potatod` or the potato daemon. This program starts a timer and outputs it to stdout.

Potatod has these options:
- `-f`
    flushing the output after each call
- `-n`
    Using `notify-send` to send notifications

## Potatoctl
The other binary from this program. This binary lists, controls and stops the potatod instances.

in all the options getting INDEX as argument, INDEX is optional, if not defined, all the instances will be toggled.

Potatoctl has this options:
- `l`
    lists all the active instances of `potatod` with their INDEX and PID.
- `-t INDEX`
    Toggle pause.
- `-p INDEX`
    Pause.
- `-u INDEX`
    Unpause.
- `-q INDEX`
    Quit.
- `-s INDEX`
    Skip to next stage. for example if its on pomodoro stage, it will be skipped to short pause.
