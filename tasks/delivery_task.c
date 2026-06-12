#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "event.h"
#include "event_masks.h"
#include "locker.h"
#include <string.h>

#include "delivery_task.h"

static QueueHandle_t delivery_queue __attribute__((section(".unprivileged_data")));

static StaticQueue_t delivery_queue_tcb 
    __attribute__((aligned(4))) 
    __attribute__((section(".unprivileged_data")));

static uint8_t delivery_queue_storage[8 * sizeof(event_t)] 
    __attribute__((aligned(4))) 
    __attribute__((section(".unprivileged_data")));

StackType_t delivery_stack[DELIVERY_STACK_SIZE_WORDS] 
    __attribute__((aligned(DELIVERY_STACK_SIZE_BYTES))) 
    __attribute__((section(".unprivileged_data")));

StaticTask_t delivery_tcb 
    __attribute__((aligned(32))) 
    __attribute__((section(".unprivileged_data")));


void delivery_task_init(void)
{
    delivery_queue = xQueueCreateStatic(
        8,
        sizeof(event_t),
        delivery_queue_storage,
        &delivery_queue_tcb
    );

    event_subscribe(
        delivery_queue,
        EVENT_BIT(EVT_DELIVERY_REQUEST)
    );
}

static void handle_delivery(event_t *evt)
{
    int id = locker_assign(evt->delivery.parcel_id);

    event_t out;

    if (id >= 0)
    {
        out.type = EVT_LOCKER_ASSIGNED;
        out.assigned.locker_id = id;

        strncpy(out.assigned.parcel_id,
                evt->delivery.parcel_id,
                PARCEL_ID_LEN);

        out.assigned.pin = 1000 + id;
    }
    else
    {
        out.type = EVT_SYSTEM_FULL;
    }

    event_publish(&out);
}


void delivery_task(void *arg)
{
    (void)arg;
    event_t evt;


    while (1)
    {       
        if (xQueueReceive(delivery_queue, &evt, portMAX_DELAY) == pdPASS)
        {
            handle_delivery(&evt);
        }
    }
}