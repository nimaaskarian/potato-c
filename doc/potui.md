% POTUI(1)
% Nima Askarian
% potui-VERSION
# NAME
potui - TUI pomodoro timer controller with todo list capabilities

# SYNOPSIS
potui

# DESCRIPTION
This has all the control options that potctl has, But its interactive and easier to deal with as a user.
It shows you the time left, pomodoro count and all the mode of the timer.
It also has a basic todo list, which has priority (1-9 and 0) and a message; Exactly like how it is in *calcurse*.
The todo list uses the *calcurse* todo data file and its format. We can't read or add notes in potui. Use calcurse if you want that.

# USAGE
there are two menus in this application. Keybindings for each is written in the following sections.
Next section has some keybinds that both menu share. its vi-like keybinds so don't worry.
## BOTH
**j**
: down

**k**
: up

**g**
: top

**G**
: top

**q**
: quit from application

**D**
: kill current (or selected) daemon

## PID SELECTION
in pid selection menu, you can select a daemon pid to control. If you have one daemon running, this menu will be skipped.

**c**
: Clear the pid file of selected daemon

## TIMER/TODOS
default location of calcurse todos file will be read and interpreted. The undone todos will be selectable. You can use the *BOTH* section keybinds to cycle between them

**Space**, **p**
: toggle pause timer

**K**
: higher todo priority (goes up in the list)

**J**
: lower todo priority (goes down in the list)

**a**
: add a new todo. You will get a *PROMPT* for your todo message and another for todo priority.

**e**
: edit the selected todo. Its very much alike the addition one.

# PROMPT
Sometimes you will be prompted for stuff. Prompt has vim mode. Escape to go to normal mode, escape again (or q) to quit.
quitting will cancel, entering will accept the prompt.
