#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <deadbeef/deadbeef.h>

#define trace(...) { fprintf(stderr, "[mericlebeef] " __VA_ARGS__); }

static DB_misc_t plugin;
static DB_functions_t *deadbeef;

typedef struct {
    pthread_t thread;
    int active;
    int duration_seconds;
    int elapsed_seconds;
    int custom_minutes;
    pthread_mutex_t mutex;
} timer_state_t;

static timer_state_t timer_state = {
    .active = 0,
    .duration_seconds = 0,
    .elapsed_seconds = 0,
    .custom_minutes = 30,
    .mutex = PTHREAD_MUTEX_INITIALIZER
};

static void *
timer_thread_func(void *arg) {
    timer_state_t *state = (timer_state_t *)arg;
    trace("Timer started: %d seconds\n", state->duration_seconds);

    while (state->active && state->elapsed_seconds < state->duration_seconds) {
        sleep(1);
        pthread_mutex_lock(&state->mutex);
        state->elapsed_seconds++;

        if (state->elapsed_seconds % 60 == 0) {
            int remaining = (state->duration_seconds - state->elapsed_seconds) / 60;
            trace("Timer: %d minutes remaining\n", remaining);
        }
        pthread_mutex_unlock(&state->mutex);
    }

    pthread_mutex_lock(&state->mutex);
    if (state->active) {
        trace("Time's up - stopping playback. Good night, Feena.\n");
        deadbeef->sendmessage(DB_EV_STOP, 0, 0, 0);
        state->active = 0;
    }
    pthread_mutex_unlock(&state->mutex);
    return NULL;
}

static int
mericlebeef_start(int minutes) {
    pthread_mutex_lock(&timer_state.mutex);

    if (timer_state.active) {
        timer_state.active = 0;
        pthread_mutex_unlock(&timer_state.mutex);
        pthread_join(timer_state.thread, NULL);
        pthread_mutex_lock(&timer_state.mutex);
    }

    timer_state.duration_seconds = minutes * 60;
    timer_state.elapsed_seconds = 0;
    timer_state.active = 1;

    trace("Starting mericlebeef timer for %d minutes\n", minutes);

    int result = pthread_create(&timer_state.thread, NULL,
                               timer_thread_func, &timer_state);

    pthread_mutex_unlock(&timer_state.mutex);

    if (result != 0) {
        trace("Failed to create timer thread\n");
        timer_state.active = 0;
        return -1;
    }
    return 0;
}

static void
mericlebeef_cancel(void) {
    pthread_mutex_lock(&timer_state.mutex);
    if (timer_state.active) {
        trace("Cancelling mericlebeef\n");
        timer_state.active = 0;
        pthread_mutex_unlock(&timer_state.mutex);
        pthread_join(timer_state.thread, NULL);
    } else {
        pthread_mutex_unlock(&timer_state.mutex);
    }
}

static int
mericlebeef_get_remaining(void) {
    pthread_mutex_lock(&timer_state.mutex);
    int remaining = 0;
    if (timer_state.active) {
        remaining = timer_state.duration_seconds - timer_state.elapsed_seconds;
    }
    pthread_mutex_unlock(&timer_state.mutex);
    return remaining;
}

static int
mericlebeef_is_active(void) {
    pthread_mutex_lock(&timer_state.mutex);
    int active = timer_state.active;
    pthread_mutex_unlock(&timer_state.mutex);
    return active;
}

static int
mericlebeef_action_custom(DB_plugin_action_t *action, int ctx) {
    int custom_mins = deadbeef->conf_get_int("mericlebeef.custom_minutes", 45);
    mericlebeef_start(custom_mins);
    return 0;
}

static int mericlebeef_action_1min(DB_plugin_action_t *action, int ctx) { mericlebeef_start(1); return 0; }
static int mericlebeef_action_15min(DB_plugin_action_t *action, int ctx) { mericlebeef_start(15); return 0; }
static int mericlebeef_action_30min(DB_plugin_action_t *action, int ctx) { mericlebeef_start(30); return 0; }
static int mericlebeef_action_60min(DB_plugin_action_t *action, int ctx) { mericlebeef_start(60); return 0; }
static int mericlebeef_action_90min(DB_plugin_action_t *action, int ctx) { mericlebeef_start(90); return 0; }
static int mericlebeef_action_cancel(DB_plugin_action_t *action, int ctx) { mericlebeef_cancel(); return 0; }

static int
mericlebeef_action_status(DB_plugin_action_t *action, int ctx) {
    if (mericlebeef_is_active()) {
        int remaining = mericlebeef_get_remaining();
        int minutes = remaining / 60;
        int seconds = remaining % 60;
        trace("Mericlebeef timer active: %d:%02d remaining\n", minutes, seconds);
    } else {
        trace("Mericlebeef inactive\n");
    }
    return 0;
}

static DB_plugin_action_t timer_1min_action = {
    .title = "Mericlebeef/1 minute",
    .name = "mericlebeef_1min",
    .flags = DB_ACTION_COMMON | DB_ACTION_ADD_MENU,
    .callback2 = mericlebeef_action_1min,
    .next = NULL
};

static DB_plugin_action_t timer_15min_action = {
    .title = "Mericlebeef/15 minutes",
    .name = "mericlebeef_15min",
    .flags = DB_ACTION_COMMON | DB_ACTION_ADD_MENU,
    .callback2 = mericlebeef_action_15min,
    .next = NULL
};

static DB_plugin_action_t timer_30min_action = {
    .title = "Mericlebeef/30 minutes",
    .name = "mericlebeef_30min",
    .flags = DB_ACTION_COMMON | DB_ACTION_ADD_MENU,
    .callback2 = mericlebeef_action_30min,
    .next = NULL
};

static DB_plugin_action_t timer_60min_action = {
    .title = "Mericlebeef/60 minutes",
    .name = "mericlebeef_60min",
    .flags = DB_ACTION_COMMON | DB_ACTION_ADD_MENU,
    .callback2 = mericlebeef_action_60min,
    .next = NULL
};

static DB_plugin_action_t timer_90min_action = {
    .title = "Mericlebeef/90 minutes",
    .name = "mericlebeef_90min",
    .flags = DB_ACTION_COMMON | DB_ACTION_ADD_MENU,
    .callback2 = mericlebeef_action_90min,
    .next = NULL
};

static DB_plugin_action_t timer_custom_action = {
    .title = "Mericlebeef/Use Preference Config",
    .name = "mericlebeef_custom",
    .flags = DB_ACTION_COMMON | DB_ACTION_ADD_MENU,
    .callback2 = mericlebeef_action_custom,
    .next = NULL
};

static DB_plugin_action_t timer_cancel_action = {
    .title = "Mericlebeef/Off",
    .name = "mericlebeef_cancel",
    .flags = DB_ACTION_COMMON | DB_ACTION_ADD_MENU,
    .callback2 = mericlebeef_action_cancel,
    .next = NULL
};

static DB_plugin_action_t timer_status_action = {
    .title = "Mericlebeef/Show Status",
    .name = "mericlebeef_status",
    .flags = DB_ACTION_COMMON | DB_ACTION_ADD_MENU,
    .callback2 = mericlebeef_action_status,
    .next = NULL
};

static DB_plugin_action_t *
mericlebeef_get_actions(DB_playItem_t *it) {
    timer_1min_action.next = &timer_15min_action;
    timer_15min_action.next = &timer_30min_action;
    timer_30min_action.next = &timer_60min_action;
    timer_60min_action.next = &timer_90min_action;
    timer_90min_action.next = &timer_custom_action;
    timer_custom_action.next = &timer_cancel_action;
    timer_cancel_action.next = &timer_status_action;
    timer_status_action.next = NULL;

    return &timer_1min_action;
}

static int
mericlebeef_plugin_start(void) {
    trace("Mericlebeef plugin started\n");
    timer_state.custom_minutes = deadbeef->conf_get_int("mericlebeef.custom_minutes", 45);
    return 0;
}

static int
mericlebeef_plugin_stop(void) {
    trace("Mericlebeef plugin stopping\n");
    mericlebeef_cancel();
    deadbeef->conf_set_int("mericlebeef.custom_minutes", timer_state.custom_minutes);
    deadbeef->conf_save();
    return 0;
}

static const char settings_dlg[] =
    "property \"Custom Timer Duration (minutes)\" spinbtn[1,1440,1] mericlebeef.custom_minutes 45;\n"
;

static DB_misc_t plugin = {
    .plugin.api_vmajor = 1,
    .plugin.api_vminor = 0,
    .plugin.version_major = 1,
    .plugin.version_minor = 0,
    .plugin.type = DB_PLUGIN_MISC,
    .plugin.id = "mericlebeef",
    .plugin.name = "Mericlebeef (Sleep Timer)",
    .plugin.descr = "Stops playback after set time",
    .plugin.copyright = "Copyright (C) 2026 mochidaz",
    .plugin.website = "https://github.com/mochidaz/mericlebeef",
    .plugin.start = mericlebeef_plugin_start,
    .plugin.stop = mericlebeef_plugin_stop,
    .plugin.configdialog = settings_dlg,
    .plugin.get_actions = mericlebeef_get_actions,
};

DB_plugin_t *
ddb_mericlebeef_load(DB_functions_t *api) {
    deadbeef = api;
    return DB_PLUGIN(&plugin);
}