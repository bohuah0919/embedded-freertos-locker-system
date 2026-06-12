#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"

#include "event.h"
#include "event_masks.h"
#include "locker.h"
#include "auto_lock.h"

static QueueHandle_t auto_lock_queue __attribute__((section(".unprivileged_data")));

static StaticQueue_t auto_lock_queue_tcb
    __attribute__((aligned(4)))
    __attribute__((section(".unprivileged_data")));

static uint8_t auto_lock_queue_storage[8 * sizeof(event_t)]
    __attribute__((aligned(4)))
    __attribute__((section(".unprivileged_data")));

StackType_t auto_lock_stack[AUTO_LOCK_STACK_SIZE_WORDS]
    __attribute__((aligned(AUTO_LOCK_STACK_SIZE_BYTES)))
    __attribute__((section(".unprivileged_data")));

StaticTask_t auto_lock_tcb
    __attribute__((aligned(32)))
    __attribute__((section(".unprivileged_data")));

void auto_lock_task_init(void)
{
    /* create subscriber queue */
    auto_lock_queue = xQueueCreateStatic(
        8,
        sizeof(event_t),
        auto_lock_queue_storage,
        &auto_lock_queue_tcb
    );

    event_subscribe(
        auto_lock_queue,
        EVENT_BIT(EVT_AUTO_LOCK_REQUEST) |
        EVENT_BIT(EVT_AUTO_LOCK_DONE)
    );

    for (int i = 0; i < MAX_LOCKERS; i++)
    {
        lockers[i].auto_lock_timer =
            xTimerCreate(
                "auto_lock",
                pdMS_TO_TICKS(AUTO_LOCK_MS),
                pdFALSE,
                (void *)i,
                auto_lock_cb
            );
    }
}

static void auto_lock_cb(TimerHandle_t timer)
{
    int id = (int) pvTimerGetTimerID(timer);

    event_t evt;
    evt.type = EVT_AUTO_LOCK_REQUEST;
    evt.locker.locker_id = id;

    event_publish(&evt);
}

void handle_auto_lock(event_t *evt)
{
    if (evt == NULL || evt->type != EVT_AUTO_LOCK_REQUEST)
    {
        return;
    }

    int id = evt->locker.locker_id;

    xSemaphoreTake(locker_mutex, portMAX_DELAY);

    hardware_lock(id);
    lockers[id].locked = pdTRUE;

    xSemaphoreGive(locker_mutex);

    event_t out;
    out.type = EVT_AUTO_LOCK_DONE;
    out.locker.locker_id = id;

    event_publish(&out);

}

void auto_lock_task(void *arg)
{
    (void)arg;

    event_t evt;

    while (1)
    {
        xQueueReceive(auto_lock_queue, &evt, portMAX_DELAY);
        handle_auto_lock(&evt);
    }
}