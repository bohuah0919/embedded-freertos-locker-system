#include "event.h"
#include "event_masks.h"
#include <string.h>
#include <stdio.h>

static event_subscriber_t subscribers[EVENT_MAX_SUBSCRIBERS] 
    __attribute__((aligned(32)))
    __attribute__((section(".unprivileged_data")));

static int subscriber_count 
    __attribute__((section(".unprivileged_data")));


void event_bus_init(void)
{
    memset(subscribers, 0, sizeof(subscribers));
    subscriber_count = 0;
}

int event_subscribe(QueueHandle_t queue, uint32_t event_mask)
{
    if (queue == NULL)
    {
        return -1;
    }

    if (subscriber_count >= EVENT_MAX_SUBSCRIBERS)
    {
        return -1;
    }

    subscribers[subscriber_count].queue = queue;
    subscribers[subscriber_count].event_mask = event_mask;
    subscriber_count++;

    return 0;
}

BaseType_t event_publish(const event_t *event)
{
    
    BaseType_t status = pdPASS;

    if (event == NULL)
    {
        return pdFAIL;
    }

    uint32_t bit = EVENT_BIT(event->type);

    for (int i = 0; i < subscriber_count; i++)
    {
        event_subscriber_t *sub = &subscribers[i];

        if ((sub->event_mask & bit) == 0)
        {
            continue;
        }

        if (xQueueSend(sub->queue, event, 0) != pdPASS)
        {
            status = pdFAIL;
        }
    }

    return status;
}