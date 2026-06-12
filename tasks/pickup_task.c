#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "event.h"
#include "event_masks.h"
#include "locker.h"
#include "pickup_task.h"

static QueueHandle_t pickup_queue __attribute__((section(".unprivileged_data")));

static StaticQueue_t pickup_queue_tcb 
    __attribute__((aligned(4)))
    __attribute__((section(".unprivileged_data")));

static uint8_t pickup_queue_storage[8 * sizeof(event_t)] 
    __attribute__((aligned(4)))
    __attribute__((section(".unprivileged_data")));

StackType_t pickup_stack[PICKUP_STACK_SIZE_WORDS] 
    __attribute__((aligned(PICKUP_STACK_SIZE_BYTES))) 
    __attribute__((section(".unprivileged_data")));

StaticTask_t pickup_tcb 
    __attribute__((aligned(32))) 
    __attribute__((section(".unprivileged_data")));

void pickup_task_init(void)
{
    pickup_queue = xQueueCreateStatic(
        8,
        sizeof(event_t),
        pickup_queue_storage,
        &pickup_queue_tcb
    );

    event_subscribe(
        pickup_queue,
        EVENT_BIT(EVT_PICKUP_REQUEST)
    );
}

static void handle_pickup(event_t *evt)
{
    int id = locker_pickup(
        evt->pickup.parcel_id,
        evt->pickup.pin
    );

    event_t out;

    if (id >= 0)
    {
        out.type = EVT_LOCKER_RELEASED;
        out.locker.locker_id = id;
    }
    else
    {
        out.type = EVT_AUTH_FAILURE;
    }

    event_publish(&out);
}

void pickup_task(void *arg)
{
    (void)arg;
    event_t evt;

    while (1)
    {
        xQueueReceive(pickup_queue, &evt, portMAX_DELAY);
        handle_pickup(&evt);
    }
}