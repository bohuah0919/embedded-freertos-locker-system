#include "locker.h"
#include "system_state.h"
#include "event.h"
#include "event_masks.h"
#include "FreeRTOS.h"
#include "timers.h"
#include <string.h>

void hardware_unlock(int id)
{

}

void hardware_lock(int id)
{

}

void locker_init(void)
{
    xSemaphoreTake(locker_mutex, portMAX_DELAY);

    for (int i = 0; i < MAX_LOCKERS; i++)
    {
        lockers[i].state = EMPTY;
        lockers[i].locked = pdTRUE;
        lockers[i].parcel_id[0] = '\0';
        lockers[i].pin = 0;
    }

    xSemaphoreGive(locker_mutex);
}

/* -----------------------------
 * ASSIGN (DELIVERY)
 * ----------------------------- */
int locker_assign(const char *parcel_id)
{
    if (!parcel_id)
        return -1;

    int id = -1;

    xSemaphoreTake(locker_mutex, portMAX_DELAY);

    for (int i = 0; i < MAX_LOCKERS; i++)
    {
        if (lockers[i].state == EMPTY)
        {
            id = i;
            lockers[i].state = OCCUPIED;

            strncpy(lockers[i].parcel_id, parcel_id, PARCEL_ID_LEN - 1);
            lockers[i].parcel_id[PARCEL_ID_LEN - 1] = '\0';
            
            lockers[i].locked = pdFALSE;
            lockers[i].pin = 1000 + id;

            xTimerReset(lockers[i].auto_lock_timer, 0);
            break;
        }
    }

    xSemaphoreGive(locker_mutex);

    if (id >= 0)
    {
        hardware_unlock(id);
    }

    return id;
}

/* -----------------------------
 * PICKUP (AUTH + ACCESS)
 * ----------------------------- */
int locker_pickup(const char *parcel_id, int pin)
{
    if (!parcel_id)
        return -1;

    int found = -1;

    xSemaphoreTake(locker_mutex, portMAX_DELAY);

    for (int i = 0; i < MAX_LOCKERS; i++)
    {
        if (lockers[i].state == OCCUPIED &&
            strncmp(lockers[i].parcel_id, parcel_id, PARCEL_ID_LEN) == 0 &&
            lockers[i].pin == pin)
        {
            found = i;

            lockers[i].state = EMPTY;
            lockers[i].parcel_id[0] = '\0';

            lockers[i].pin = 0;

            lockers[i].locked = pdFALSE;
            xTimerReset(lockers[i].auto_lock_timer, 0);

            break;
        }
    }

    xSemaphoreGive(locker_mutex);

    if (found >= 0)
    {
        hardware_unlock(found);
    }

    return found;
}
