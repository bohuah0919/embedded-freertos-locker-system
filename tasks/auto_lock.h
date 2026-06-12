#ifndef AUTO_LOCK_H
#define AUTO_LOCK_H

#include "FreeRTOS.h"
#include "timers.h"
#include "locker.h"

#include <stdint.h>

#define AUTO_LOCK_MS 3000
#define AUTO_LOCK_STACK_SIZE_WORDS 1024
#define AUTO_LOCK_STACK_SIZE_BYTES (AUTO_LOCK_STACK_SIZE_WORDS * 4)

void auto_lock_task_init(void);

static void auto_lock_cb(TimerHandle_t timer);

void handle_auto_lock(event_t *evt);

void auto_lock_task(void *arg);

#endif