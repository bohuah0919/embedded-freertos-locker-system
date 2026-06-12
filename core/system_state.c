#include "system_state.h"

static StaticSemaphore_t locker_mutex_buffer;

locker_t lockers[MAX_LOCKERS];
SemaphoreHandle_t locker_mutex;

void system_state_init(void)
{
    locker_mutex = xSemaphoreCreateMutexStatic(&locker_mutex_buffer);

    for (int i = 0; i < MAX_LOCKERS; i++)
    {
        lockers[i].state = EMPTY;
        lockers[i].parcel_id[0] = '\0';
        lockers[i].pin = 0;
    }
}