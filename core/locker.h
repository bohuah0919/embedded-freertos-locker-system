#ifndef LOCKER_H
#define LOCKER_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "timers.h"
#include <stdint.h>

#define MAX_LOCKERS 16
#define PARCEL_ID_LEN 32

/* -----------------------------
 * LOGICAL STATE (ownership only)
 * ----------------------------- */
typedef enum {
    EMPTY = 0,
    OCCUPIED
} locker_state_t;

/* -----------------------------
 * LOCKER DATA MODEL
 * ----------------------------- */
typedef struct
{
    locker_state_t state;

    char parcel_id[PARCEL_ID_LEN];
    int pin;

    BaseType_t locked;

    TimerHandle_t auto_lock_timer;

} locker_t;

extern locker_t lockers[MAX_LOCKERS];
extern SemaphoreHandle_t locker_mutex;

int locker_assign(const char *parcel_id);
int locker_pickup(const char *parcel_id, int pin);

void hardware_lock(int id);
void hardware_unlock(int id);

void system_state_init(void);

int find_empty_locker(void);

#endif