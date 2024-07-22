/**
 * @file soft_timer.c
 * @author monster
 * @date 2024/4/3
 */

/*********************
 *      INCLUDES
 *********************/
#include "soft_timer.h"
#include "string.h"


/*********************
 *      DEFINES
 *********************/
#define IDLE_MEAS_PERIOD 500 /*[ms]*/

/**********************
 *      TYPEDEFS
 **********************/

/**
 * Descriptor of a soft_timer linked list
 */
typedef struct {
    soft_timer_t *head;
    soft_timer_t *tail;
    uint32_t     size;
} soft_timer_ll_t;

/**
 * Descriptor of a soft_timer state
 */
typedef struct {
    uint8_t created: 1;
    uint8_t deleted: 1;
    uint8_t auto_del: 1;

    uint32_t next_call_time;    /*next timer exec callback time*/
    uint32_t busy_time;         /*timer callback time*/
    uint32_t idle_period_time;  /*last idle period time*/
    uint32_t idle_pct;          /*timer idle time percentage*/
} soft_timer_state_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static soft_timer_ll_t    soft_timer_ll;
static soft_timer_state_t timer_state;

static soft_timer_get_tick_cb_t _get_tick;

#if SOFT_TIMER_STATIC_STORAGE
static soft_timer_t timer_buffer[SOFT_TIMER_CUSTOM_COUNT];
#endif

/**********************
 *  STATIC PROTOTYPES
 **********************/
static uint32_t tick_elaps(uint32_t prev_tick);
static int32_t soft_timer_exec(soft_timer_t *timer);
static uint32_t soft_timer_time_remaining(soft_timer_t *timer);
static uint32_t soft_timer_get_time();

/**********************
 *      MACROS
 **********************/
#if SOFT_TIMER_LOG_EN
#   define SOFT_TIMER_ASSERT(fmt, ...)  SOFT_TIMER_CUSTOM_LOG("[ST] " fmt "\r\n", ##__VA_ARGS__)
#else
#   define SOFT_TIMER_ASSERT(fmt, ...)
#endif


/**********************
 *   GLOBAL FUNCTIONS
 **********************/


/**
* @brief initialize soft timer, should be before using timer.
*/
int32_t soft_timer_init(soft_timer_get_tick_cb_t get_tick) {
    soft_timer_ll.head = NULL;
    soft_timer_ll.head = NULL;
    soft_timer_ll.size = 0;

    memset(&timer_state, 0, sizeof(soft_timer_state_t));

#if SOFT_TIMER_STATIC_STORAGE
    memset(timer_buffer, 0, sizeof(timer_buffer));
#endif

#ifdef SOFT_TIMER_COMPLET_DEL
    timer_state.auto_del = SOFT_TIMER_COMPLET_DEL;
#endif

    if (get_tick != NULL) {
        _get_tick = get_tick;
    } else {
        _get_tick = soft_timer_get_time;
    }

    return 0;
}

uint32_t soft_timer_handler(void) {
    //SOFT_TIMER_ASSERT("begin");

    uint32_t tick_start = _get_tick();

    /*Check 'tick' is available*/
    if (tick_start == 0) {
        static uint32_t run_cnt = 0;
        run_cnt = (run_cnt + 1) % 100;
        if (run_cnt == 0) {
            SOFT_TIMER_ASSERT("It seems '_get_tick()' is not available.");
        }
    }

    timer_state.created = false;
    timer_state.deleted = false;
    /*Run all timer form the linked list*/
    soft_timer_t *active = soft_timer_ll.head;

    while (active) {
        soft_timer_exec(active);
        if (timer_state.created || timer_state.deleted) {
            SOFT_TIMER_ASSERT("timer was created or deleted");
            break;
        }
        active = active->next;
    };

    /*Get next timer callback remaining time*/
    active = soft_timer_ll.head;
    uint32_t next_call_time = UINT32_MAX;
    uint32_t time_remain    = 0;
    while (active) {
        if (!active->paused) {
            time_remain = soft_timer_time_remaining(active);
            if (time_remain < next_call_time) next_call_time = time_remain;
        }

        active = active->next;
    }
    timer_state.next_call_time = next_call_time;

    /*Get idle time PCT*/
    timer_state.busy_time += tick_elaps(tick_start);
    uint32_t idle_time = tick_elaps(timer_state.idle_period_time);
    if (idle_time >= IDLE_MEAS_PERIOD) {
        /*Calculate the busy percentage*/
        timer_state.idle_pct         = (timer_state.busy_time * 100) / idle_time;
        timer_state.idle_pct         = timer_state.idle_pct > 100 ? 0 :
                                       (timer_state.idle_pct == 100 ? 100 : 100 - timer_state.idle_pct);
        timer_state.busy_time        = 0;
        timer_state.idle_period_time = _get_tick();
    }

    //SOFT_TIMER_ASSERT("finished (next timer call after %u ms)", next_call_time);

    return next_call_time;
}

/**
 * Create a new lv_timer
 * @param period call period in ms unit
 * @param timer_xcb a callback to call periodically.
 *                 (the 'x' in the argument name indicates that it's not a fully generic function because it not follows
 *                  the `func_name(object, callback, ...)` convention)
 * @param user_data custom parameter
 * @return pointer to the new timer
 */
soft_timer_t *soft_timer_create(uint32_t period, soft_timer_cb_t timer_xcb, void *user_data) {
    soft_timer_t *new_timer = NULL;

#if SOFT_TIMER_CUSTOM_COUNT
    if (soft_timer_ll.size >= SOFT_TIMER_CUSTOM_COUNT) {
        SOFT_TIMER_ASSERT("Timer created the upper limit.");
        return NULL;
    }
#endif

#if SOFT_TIMER_STATIC_STORAGE
    for (uint32_t i = 0; i < SOFT_TIMER_CUSTOM_COUNT; i++) {
        if (timer_buffer[i].timer_cb == NULL) {
            new_timer = &timer_buffer[i];
            break;
        }
    }
#else
    new_timer = SOFT_TIMER_CUSTOM_MALLOC(sizeof(soft_timer_t));
#endif

    if (new_timer == NULL) return NULL;

    timer_state.created = true;

    new_timer->period       = period;
    new_timer->timer_cb     = timer_xcb;
    new_timer->user_data    = user_data;
    new_timer->repeat_count = -1;
    new_timer->paused       = 0;
    new_timer->last_run     = _get_tick();
    new_timer->last         = soft_timer_ll.tail;
    new_timer->next         = NULL;

    /*If there is old tail then later it goes the new*/
    if (soft_timer_ll.tail != NULL) {
        soft_timer_ll.tail->next = new_timer;
    }

    /*If there is no head (1. node) set the head too*/
    if (soft_timer_ll.head == NULL) {
        soft_timer_ll.head = new_timer;
    }

    /*Set the new tail in the linked list.*/
    soft_timer_ll.tail = new_timer;
    soft_timer_ll.size += 1;

    return new_timer;
}

/**
 * Delete a sotf_timer
 * @param timer pointer to an soft_timer
 */
void soft_timer_del(soft_timer_t *timer) {
    if (timer == NULL) return;

    timer_state.deleted = true;

    if (soft_timer_ll.head == timer) {
        /*The new head will be the node after 'timer'*/
        soft_timer_ll.head = timer->next;
        if (soft_timer_ll.head == NULL) {
            soft_timer_ll.tail = NULL;
        } else {
            soft_timer_ll.head->last = NULL;
        }
    } else if (soft_timer_ll.tail == timer) {
        /*The new tail will be the node before 'timer'*/
        soft_timer_ll.tail = timer->last;
        if (soft_timer_ll.tail == NULL) {
            soft_timer_ll.head = NULL;
        } else {
            soft_timer_ll.tail->next = NULL;
        }
    } else {
        soft_timer_t *last = timer->last;
        soft_timer_t *next = timer->next;

        last->next = next;
        next->last = last;
    }

    soft_timer_ll.size += (soft_timer_ll.size > 0) ? -1 : 0;

#if SOFT_TIMER_STATIC_STORAGE
    memset(timer, 0, sizeof(soft_timer_t));
#else
    SOFT_TIMER_CUSTOM_FREE(timer);
#endif
}

/**
 * Pause/Resume a timer.
 * @param timer pointer to an soft_timer
 */
void soft_timer_pause(soft_timer_t *timer) {
    timer->paused = true;
}

void soft_timer_resume(soft_timer_t *timer) {
    timer->paused = false;
}

void soft_timer_ready(soft_timer_t *timer) {
    timer->last_run = _get_tick() - timer->period - 1;
}

void soft_timer_reset(soft_timer_t *timer) {
    timer->last_run = _get_tick();
}

void soft_timer_set_period(soft_timer_t *timer, uint32_t period) {
    timer->period = period;
}

void soft_timer_set_cb(soft_timer_t *timer, soft_timer_cb_t timer_xcb) {
    timer->timer_cb = timer_xcb;
}

void soft_timer_set_repeat_count(soft_timer_t *timer, int32_t repeat_count) {
    timer->repeat_count = repeat_count;
}

uint32_t soft_timer_get_idle_ptc(void) {
    return timer_state.idle_pct;
}

/**
 * Get the elapsed milliseconds since a previous time stamp
 * @param prev_tick a previous time stamp (return value of lv_tick_get() )
 * @return the elapsed milliseconds since 'prev_tick'
 */
static uint32_t tick_elaps(uint32_t prev_tick) {
    uint32_t act_time = _get_tick();

    if (act_time < prev_tick) {
        return (UINT32_MAX - prev_tick + 1) + act_time;
    }

    return act_time - prev_tick;
}

static int32_t soft_timer_exec(soft_timer_t *timer) {
    if (!timer) return 1;

    /*timer running check*/
    if (timer->paused) return 1;

    if (soft_timer_time_remaining(timer) == 0) {
        int32_t original_repeat_count = timer->repeat_count;

        timer->repeat_count += (timer->repeat_count > 0) ? -1 : 0;
        timer->last_run               = _get_tick();
        SOFT_TIMER_ASSERT("calling timer callback: %p", *((void **) &timer->timer_cb));
        if (timer->timer_cb && original_repeat_count != 0) timer->timer_cb(timer);
        SOFT_TIMER_ASSERT("timer callback: %p finished", *((void **) &timer->timer_cb));
    }

    if (timer_state.deleted || timer_state.created) {
        SOFT_TIMER_ASSERT("timer was created or deleted");
        return 0;
    }

    if (timer->repeat_count == 0) {
        if (timer_state.auto_del) {
            SOFT_TIMER_ASSERT("deleting timer with %p", *((void **) &timer->timer_cb));
            soft_timer_del(timer);
        } else {
            SOFT_TIMER_ASSERT("pausing timer with %p", *((void **) &timer->timer_cb));
            soft_timer_pause(timer);
        }
    }

    return 0;
}

/**
 * Find out how much time remains before a timer must be run.
 * @param timer pointer to soft_timer
 * @return the time remaining, or 0 if it needs to be run again
 */
static uint32_t soft_timer_time_remaining(soft_timer_t *timer) {

    uint32_t elaps = tick_elaps(timer->last_run);
    if (elaps >= timer->period)
        return 0;

    return timer->period - elaps;
}

static uint32_t soft_timer_get_time() {
    return SOFT_TIMER_CUSTOM_GET_TICK;
}