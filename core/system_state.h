#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "locker.h"

#include <stdint.h>

extern locker_t lockers[MAX_LOCKERS];
extern SemaphoreHandle_t locker_mutex;

void system_state_init(void);

#endif