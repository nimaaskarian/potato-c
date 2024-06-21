#include "include/utils.h"
#include "include/timer.h"

static const char * notification_format = "herbe '%t' '%d' &> /dev/null & disown";

// Set to {NULL, NULL} If you want program to have no notifications on event
static const notif_t pomodoro_notif     = {.title = "Pomodoro",     .body = "Time to focus!"};
static const notif_t short_break_notif  = {.title = "Short pause",  .body = "Have a little time for yourself."};
static const notif_t long_break_notif   = {.title = "Long pause",   .body = "Take a good long break."};
static const notif_t paused_notif       = {.title = "Paused",       .body = "Timer have been paused."};
static const notif_t unpaused_notif     = {.title = "Unpaused",     .body = "Timer have been unpaused."};

static const char * pomodoro_before_time = "PM";
static const char * short_break_before_time = "SB";
static const char * long_break_before_time = "LB";

static const char * timer_format = "%b %t P %p";

#define TO_NULL " &> /dev/null"
static const char* ON_POMODORO_START_COMMANDS[] = {
  CONFIG_DIR "/on-pomodoro.sh" TO_NULL
};
static const char* ON_SHORT_BREAK_START_COMMANDS[] = {
  CONFIG_DIR "/on-short-break.sh" TO_NULL
};
static const char* ON_LONG_BREAK_START_COMMANDS[] = {
  CONFIG_DIR "/on-long-break.sh" TO_NULL
};
static const char* ON_PAUSE_COMMANDS[] = {
  CONFIG_DIR "/on-pause.sh" TO_NULL
};
static const char* ON_UNPAUSE_COMMANDS[] = {
  CONFIG_DIR "/on-unpause.sh" TO_NULL
};

#define default_timer (Timer){\
.initial_pomodoro_count = 4,\
.pomodoro_minutes = 25,\
.short_break_minutes = 5,\
.long_break_minutes = 30 \
}
