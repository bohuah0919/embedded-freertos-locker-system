#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdio.h>

#include "event.h"
#include "event_masks.h"
#include "logger_task.h"

static QueueHandle_t logger_queue __attribute__((section(".unprivileged_data")));

static StaticQueue_t logger_queue_tcb 
    __attribute__((aligned(4)))
    __attribute__((section(".unprivileged_data")));

static uint8_t logger_queue_storage[16 * sizeof(event_t)] 
    __attribute__((aligned(4)))
    __attribute__((section(".unprivileged_data")));

StackType_t logger_stack[LOGGER_STACK_SIZE_WORDS] 
    __attribute__((aligned(LOGGER_STACK_SIZE_BYTES))) 
    __attribute__((section(".unprivileged_data")));

StaticTask_t logger_tcb 
    __attribute__((aligned(32))) 
    __attribute__((section(".unprivileged_data")));

void logger_task_init(void)
{
    logger_queue = xQueueCreateStatic(
        16,
        sizeof(event_t),
        logger_queue_storage,
        &logger_queue_tcb
    );

    event_subscribe(
        logger_queue,
        EVENT_BIT(EVT_DELIVERY_REQUEST) |
        EVENT_BIT(EVT_PICKUP_REQUEST) |
        EVENT_BIT(EVT_LOCKER_ASSIGNED) |
        EVENT_BIT(EVT_LOCKER_RELEASED) |
        EVENT_BIT(EVT_SYSTEM_FULL) |
        EVENT_BIT(EVT_AUTH_FAILURE) |
        EVENT_BIT(EVT_AUTO_LOCK_REQUEST) |
        EVENT_BIT(EVT_AUTO_LOCK_DONE)
    );
}

static void print_locker_visualization(void)
{
    printf("\n--- LOCKER STATUS MAP -------------------\n");
    
    /* Take the mutex before reading global locker states to ensure thread-safety */
    if (xSemaphoreTake(locker_mutex, pdMS_TO_TICKS(100)) == pdPASS)
    {
        for (int i = 0; i < 16; i++)
        {
            /* Print locker label and status icon: 
             * [L00:L] = Locked/Occupied, [L00:U] = Unlocked/Available */
            printf("[L%02d:%c|%c] ", 
                i, 
                lockers[i].locked ? 'L' : 'U', 
                lockers[i].state == EMPTY?'E':'O');

            /* Break into rows of 4 for a clean square grid layout */
            if ((i + 1) % 4 == 0)
            {
                printf("\n");
            }
        }
        xSemaphoreGive(locker_mutex);
    }
    else
    {
        printf("[Logger] Status map temporarily unavailable (Busy)\n");
    }
    printf("-----------------------------------------\n\n");
}

void logger_task(void *arg)
{
    (void)arg;
    event_t evt;

    while (1)
    {
        xQueueReceive(logger_queue, &evt, portMAX_DELAY);

        switch (evt.type)
        {
            case EVT_DELIVERY_REQUEST:
                printf("DELIVERY: %s\n",
                       evt.delivery.parcel_id);
                break;

            case EVT_PICKUP_REQUEST:
                printf("PICKUP: %s PIN:%d\n",
                       evt.pickup.parcel_id,
                       evt.pickup.pin);
                break;

            case EVT_LOCKER_ASSIGNED:
                printf("ASSIGNED: L%2d %s PIN:%d\n",
                       evt.assigned.locker_id,
                       evt.assigned.parcel_id,
                       evt.assigned.pin);
                break;

            case EVT_LOCKER_RELEASED:
                printf("RELEASED: L%2d\n",
                       evt.locker.locker_id);
                break;

            case EVT_SYSTEM_FULL:
                printf("SYSTEM FULL\n");
                break;

            case EVT_AUTH_FAILURE:
                printf("AUTH FAILED\n");
                break;
            
            case EVT_AUTO_LOCK_REQUEST:
                printf("AUTO-LOCK REQUEST: L%2d\n",
                       evt.locker.locker_id);
                break;

            case EVT_AUTO_LOCK_DONE:
                printf("AUTO-LOCK DONE: L%2d\n",
                       evt.locker.locker_id);
                break;

            default:
                break;
        }

        print_locker_visualization();

    }

    
}