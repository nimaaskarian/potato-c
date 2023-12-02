<div align="center">

# Potato-c 
![GitHub top language](https://img.shields.io/github/languages/top/nimaaskarian/potato-c?color=orange)
![AUR version](https://img.shields.io/aur/version/potato-c?logo=archlinux)

A no BS pomodoro timer for GNU/Linux operating systems.

[Getting started](#getting-started) •
[Installation](#installation) •
[Configuration](#configuration) •
[Integrations](#third-party-integrations)

https://github.com/MiliAxe/potato-c/assets/88832088/c448536a-d717-49f1-bc0d-7a7d65235c1b

</div>

## Introduction
The Pomodoro method has made me very efficient and fast when doing my daily tasks, but all the pomodoro timer applications that existed, sucked in a way in my opinion. So I started Potato-c. Potato-c is a server-client application, inspired by many similar applications like mpd-mpc-ncmpc, I tried to keep the application simple, modular and as efficiant on resources as possible, so its kinda inspired by dwm and other [suckless](https://suckless.org) tools. But this doesn't make the application miss on any features that a pomodoro timer may need.

## The Pomodoro method
This method is about breaking your tasks into small intervals so you don't get zoned out from not having enough TikTok videos. This can be very good for homeworks and tasks of your job, University or school that you are not really into.

## Why potato-c
This program's base idea is from [Tomato.C](https://github.com/gabrielzschmitz/Tomato.C), "a pure C pomodoro timer",
which after trying to fork it, the ideas behind it seemed problematic to me.  
This program tries to be able to run on a potato (as the name suggests)
- Blazingly fast and efficient
- Is modular and simple
- It is based on POSIX Philosophy, does one thing and one thing only.

## Installation 
### Arch Linux (AUR)
You can install [potato-c from the AUR](https://aur.archlinux.org/packages/potato-c).

You may use an AUR helper like yay to automate the process:
```
yay -S potato-c
```
### Manually building
You can use make/git to build and install the package for yourself like so:
```shell
git clone https://github.com/nimaaskarian/potato-c
cd potato-c
sudo make install
```

## Getting started
Simply put, this program consists of three binaries. Each intended for their own purposes:
- **potd:** the daemon responsible for a pomodoro session. Managing the timers and keeping
track of the progress.
- **potctl:** the interface to interacting with the timers using commandline.
- **potui:** the TUI to manage your timers with to-do lists.

In order to quickly set up a timer, do the following:
```bash
potd                # Starts a potato-c daemon (writes current remaining time to stdout by default)

potctl              # Get the status of the daemon
potctl -p           # Pauses the timer            
potctl -u           # Unpauses the timer            
potctl -t           # Toggle pauses the timer            
potctl -r           # Resets the timer             

potui               # Launch the TUI
```
For more detailed information on how to use the program refer to [usage](#usage).

## Configuration
For more configuration, you may edit the header file `config.h` (in suckless fashion) which is created after building the
project. All the definitions have reasonable names and are easy to understand.

Make sure to recompile the project once you edit the configuration file for the changes to take
effect:

```
sudo make clean install
```

## Usage
Some features that potato-c has, and you may want to use them.
### Controlling (Pausing, etc)
Even though the `potd` (potato-deamon) itself isn't able to get any inputs from the user, it can be controlled by `potctl` and `potui`. This control can be pausing, skipping, resetting, adding to timer, adding to pomodoros etc.

### To-do list
`potui` binary has a to-do list. The to-do list data is from [calcurse](https://www.calcurse.org/)'s to-do files; But potato-c doesn't depend on calcurse by any means. The to-do capabilites are limited compared to calcurse, for example you can't add notes to your todos with potato-c, but notes can be added and viewed with calcurse if you want it.

### Commands on events
Specify a list of commands in config.h that will be ran for each event (pomodoro start, short break start, long break start).

### Using another computer as your server
You can use another computer in your local network as your pomodoro server. You can't really control it, but you can view the timer. Refer to `man potctl` for further details.

If you want more information, please refer to the man pages. If I do a documentation in here, I will likely forget to update it.
Just after installation, use the commands below to read the docs.
```shell
man potd
man potctl
man potui
```

## Integration
Since this program is modular and simple, adding information about timers to your bars is a 
straight-forward process:

### Polybar
```ini
[module/potato]
type = custom/script
exec = "potd -f"
tail = true
```

### Waybar
```json
"custom/potato": {
    "exec": "potd -fN"
},
```

This is of course, a very minimal implementation. You could add more with click events and `potctl` to make the bar module work like a charm.

## Contributing
Code as modular as possible. I'm still learning, so thats the only suggestion and rule I can have for contributors.
