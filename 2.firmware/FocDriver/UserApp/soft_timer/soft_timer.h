/**
 * @file soft_timer.h
 * @author monster
 * @date 2024/4/3
 */

#ifndef SOFT_TIMER_H
#define SOFT_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/*********************
 *      DEFINES
 *********************/
#define SOFT_TIMER_LOG_EN           0       /*Inside log enable, 0: diable; 1: enable*/
#define SOFT_TIMER_COMPLET_DEL      1       /*Auto delete oneself after complet timer callback, 0: pause, 1: delete*/
#define SOFT_TIMER_TICK_CUSTOM      1       /*Allow soft_timer used external 'SOFT_TIMER_CUSTOM_GET_TICK' function, 0: not allow; 1: allow*/
#define SOFT_TIMER_STATIC_STORAGE   1       /*Used static storage area, 0: malloc cache��1: static cache*/
#define SOFT_TIMER_CUSTOM_COUNT     (16)    /*Timer max count*/

/**********************
 *      MACROS
 **********************/

#define SOFT_TIMER_CUSTOM_MALLOC    malloc
#define SOFT_TIMER_CUSTOM_FREE      free
#define SOFT_TIMER_CUSTOM_REALLOC   realloc

#if SOFT_TIMER_LOG_EN
#   include <stdio.h>
#   define SOFT_TIMER_CUSTOM_LOG    printf
#endif

/*Use a custom tick source that tells the elapsed time in milliseconds.*/
#if SOFT_TIMER_TICK_CUSTOM
#if defined(_WIN32)
#   define SOFT_TIMER_TICK_CUSTOM_INCLUDE <Windows.h>

#   include SOFT_TIMER_TICK_CUSTOM_INCLUDE

#   pragma comment(lib, "Winmm.lib")
#   define SOFT_TIMER_CUSTOM_GET_TICK (timeGetTime())
#else
#   define SOFT_TIMER_CUSTOM_GET_TICK (0)
#endif
#else
#   define SOFT_TIMER_CUSTOM_GET_TICK (0)
#endif

/**********************
 *      TYPEDEFS
 **********************/
struct _soft_timer_t;

/**
 * Timers execute this type of functions.
 */
typedef void (*soft_timer_cb_t)(struct _soft_timer_t *);

/**
 * Timer get tick function.
 */
typedef uint32_t (*soft_timer_get_tick_cb_t)(void);

/**
 * Descriptor of a soft_timer
 */
typedef struct _soft_timer_t {
    const char *name;      /**< Timer name*/
    uint32_t   period;     /**< How often the timer should run*/
    uint32_t   last_run;   /**< Last time the timer ran*/
    void       *user_data; /**< Custom user data*/
    int32_t    repeat_count;/**< -1: infinity; 1: One time; n>0: residual times*/
    uint32_t   paused: 1;

    soft_timer_cb_t      timer_cb;   /**< Timer function*/
    struct _soft_timer_t *last; /**< Last timer node*/
    struct _soft_timer_t *next; /**< Next timer node*/
} soft_timer_t;


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize timer linked list
 */
int32_t soft_timer_init(soft_timer_get_tick_cb_t get_tick);


/**
 * Call it periodically to handle soft_timer.
 * @return time till it needs to be run next (in ms)
 */
uint32_t soft_timer_handler(void);


/**
 * Create a new soft_timer
 * @param period call period in ms unit
 * @param timer_xcb a callback to call periodically.
 *                 (the 'x' in the argument name indicates that it's not a fully generic function because it not follows
 *                  the `func_name(object, callback, ...)` convention)
 * @param user_data custom parameter
 * @return pointer to the new timer
 */
soft_timer_t *soft_timer_create(uint32_t period, soft_timer_cb_t timer_xcb, void *user_data);

/**
 * Delete a sotf_timer
 * @param pointer to a soft_timer
 */
void soft_timer_del(soft_timer_t *timer);

/**
 * Pause/Resume a timer.
 * @param pointer to a soft_timer
 */
void soft_timer_pause(soft_timer_t *timer);

void soft_timer_resume(soft_timer_t *timer);

/**
 * Make a soft_timer ready. It will not its period.
 * @param pointer to a soft_timer
 */
void soft_timer_ready(soft_timer_t *timer);

/**
 * Reset a soft_timer.
 * It will be called the previously set period milliseconds later.
 * @param pointer to a soft_timer
 */
void soft_timer_reset(soft_timer_t *timer);

/**
 * Set new period for a soft_timer
 * @param timer pointer to a soft_timer
 * @param period the new period
 */
void soft_timer_set_period(soft_timer_t *timer, uint32_t period);

/**
 * Set the callback the timer (the function to call periodically)
 * @param timer pointer to a soft_timer
 * @param timer_cb the function to call periodically
 */
void soft_timer_set_cb(soft_timer_t *timer, soft_timer_cb_t timer_xcb);

/**
 * Set the number of times a timer will repeat.
 * @param timer pointer to a lv_timer.
 * @param repeat_count -1: infinity;  0: stop;  n>0: residual times
 */
void soft_timer_set_repeat_count(soft_timer_t *timer, int32_t repeat_count);


/**
 * Get idle percentage
 * @return the soft_timer idle in percentage
 */
uint32_t soft_timer_get_idle_ptc(void);


#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif //SOFT_TIMER_H
