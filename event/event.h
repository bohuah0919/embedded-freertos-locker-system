#ifndef EVENT_H
#define EVENT_H

#include "FreeRTOS.h"
#include "queue.h"
#include "locker.h"
#include <stdint.h>

#define EVENT_MAX_SUBSCRIBERS 8
#define EVENT_QUEUE_DEPTH     16

typedef enum
{
    EVT_NONE = 0,

    EVT_DELIVERY_REQUEST,
    EVT_PICKUP_REQUEST,

    EVT_LOCKER_ASSIGNED,
    EVT_LOCKER_RELEASED,

    EVT_AUTH_FAILURE,

    EVT_SYSTEM_FULL,
    EVT_SYSTEM_AVAILABLE,

    EVT_AUTO_LOCK_REQUEST,
    EVT_AUTO_LOCK_DONE

} event_type_t;

/* ================= Payloads ================= */

typedef struct
{
    char parcel_id[PARCEL_ID_LEN];
} delivery_request_t;

typedef struct
{
    char parcel_id[PARCEL_ID_LEN];
    int pin;
} pickup_request_t;

typedef struct
{
    int locker_id;
} locker_event_t;

typedef struct
{
    int locker_id;

    char parcel_id[PARCEL_ID_LEN];

    int pin;
} locker_assigned_t;

/* ================= Event ================= */

typedef struct
{
    event_type_t type;

    union
    {
        delivery_request_t delivery;

        pickup_request_t pickup;

        locker_event_t locker;

        locker_assigned_t assigned;

    };

} event_t;

/* ================= Subscriber ================= */

typedef struct
{
    QueueHandle_t queue;

    uint32_t event_mask;

} event_subscriber_t;

/* ================= API ================= */

void event_bus_init(void);

int event_subscribe(QueueHandle_t queue, uint32_t event_mask);

BaseType_t event_publish(const event_t *event);

#endif