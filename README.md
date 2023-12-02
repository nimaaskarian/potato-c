<div align="center">

# Potato-c 
![GitHub top language](https://img.shields.io/github/languages/top/nimaaskarian/potato-c?color=orange)
![AUR version](https://img.shields.io/aur/version/potato-c?logo=archlinux)

A no BS pomodoro timer for GNU/Linux operating systems.

[Getting started](#getting-started) •
[Installation](#installation) •
[Configuration](#configuration) •
[Integrations](#third-party-integrations)

% Include a video of it in action in polybar (low height video) %

% Include a video of it in action in waybar (low height video)%

% Include a video of it in action in dwm (low height video)%
</div>

## Introduction
% Write briefly about what inspired you to write potato-c %

## The Pomodoro method
This method is about breaking your tasks into small intervals so you don't get zoned out from not having enough TikTok videos.

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
Refer to man pages for full documentation. If I do a documentation in here, I will likely forget to update it.
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
% Write instructions on how to edit the configuration %
% Include a video/picture of it in action %

### Waybar
% Write instructions on how to edit the configuration %
% Include a video/picture of it in action %

### dwmblocks
% Write instructions on how to edit the configuration %
% Include a video/picture of it in action %

## Contributing
% You may want to write notes about people who want to contribute to the project %
