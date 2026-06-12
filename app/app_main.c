#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

#include "event.h"
#include "system_state.h"
#include "locker.h"

#include "delivery_task.h"
#include "pickup_task.h"
#include "logger_task.h"
#include "auto_lock.h"
#include "cli_task.h"

#define BOOT_STACK_SIZE_WORDS 1024
#define BOOT_STACK_SIZE_BYTES (BOOT_STACK_SIZE_WORDS * sizeof(StackType_t))

static StackType_t xBootTaskStack[BOOT_STACK_SIZE_WORDS] __attribute__((aligned(BOOT_STACK_SIZE_BYTES)));
static void vSystemBootTask(void *pvParameters);

extern StackType_t delivery_stack[];
extern StaticTask_t delivery_tcb;

extern StackType_t pickup_stack[];
extern StaticTask_t pickup_tcb;

extern StackType_t logger_stack[];
extern StaticTask_t logger_tcb;

extern StackType_t auto_lock_stack[];
extern StaticTask_t auto_lock_tcb;

extern StackType_t cli_stack[];
extern StaticTask_t cli_tcb;

void test_task(void *arg);

void app_main(void)
{
    printf("Spawning Privileged Boot Task...\n");

    TaskParameters_t xBootTaskParams = {
        .pvTaskCode     = vSystemBootTask,
        .pcName         = "SysBoot",
        .usStackDepth   = BOOT_STACK_SIZE_WORDS,
        .pvParameters   = NULL,
        .uxPriority     = (configMAX_PRIORITIES - 1) | portPRIVILEGE_BIT, 
        .puxStackBuffer = xBootTaskStack,
        .xRegions       = { 
            { NULL, 0, 0 },
            { NULL, 0, 0 },
            { NULL, 0, 0 }
        }
    };

    BaseType_t xResult = xTaskCreateRestricted(&xBootTaskParams, NULL);

    if (xResult != pdPASS)
    {
        printf("Critical Error: Boot Task creation failed\n");
        for(;;);
    }

    printf("Starting MPU Scheduler. SVC vectors arming now...\n");
    vTaskStartScheduler();

    printf("Scheduler returned unexpectedly\n");
    for (;;);
}

static void vSystemBootTask(void *pvParameters)
{
    (void)pvParameters;

    printf("Boot Task Hook: MPU active, privileges granted.\n");

    /* Core peripheral subsystem setup */
    system_state_init();
    locker_init();
    event_bus_init();

    delivery_task_init();
    pickup_task_init();
    logger_task_init();
    auto_lock_task_init();

    TaskHandle_t xDeliveryTaskHandle = xTaskCreateStatic(
        delivery_task,
        "delivery",
        DELIVERY_STACK_SIZE_WORDS,
        NULL,
        3 | portPRIVILEGE_BIT, /* Retain your execution privilege */
        delivery_stack,
        &delivery_tcb
    );
    

    TaskHandle_t xPickupTaskHandle = xTaskCreateStatic(
        pickup_task,
        "pickup",
        PICKUP_STACK_SIZE_WORDS,
        NULL,
        3 | portPRIVILEGE_BIT, /* Priority 3 (Matches delivery), Privileged */
        pickup_stack,
        &pickup_tcb
    );

    TaskHandle_t xLoggerTaskHandle = xTaskCreateStatic(
        logger_task,
        "logger",
        LOGGER_STACK_SIZE_WORDS,
        NULL,
        2 | portPRIVILEGE_BIT, /* Priority 2 (Lower priority to handle logs in background) */
        logger_stack,
        &logger_tcb
    );

    TaskHandle_t xAutoLockTaskHandle = xTaskCreateStatic(
        auto_lock_task,
        "auto_lock",
        AUTO_LOCK_STACK_SIZE_WORDS,
        NULL,
        2 | portPRIVILEGE_BIT,   /* Priority 2 (same as pickup/delivery) */
        auto_lock_stack,
        &auto_lock_tcb
    );


    TaskHandle_t xCliTaskHandle = xTaskCreateStatic(
        cli_task,
        "cli_task",
        1024, 
        NULL, 
        (tskIDLE_PRIORITY + 1) | portPRIVILEGE_BIT,
        cli_stack,
        &cli_tcb 
    );

    printf("System up and running. Transitioning Boot Task to background...\n");

    vTaskDelete(NULL);
}


/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask,
                                    char * pcTaskName )
{
    /* If configCHECK_FOR_STACK_OVERFLOW is set to either 1 or 2 then this
     * function will automatically get called if a task overflows its stack. */
    ( void ) pxTask;
    ( void ) pcTaskName;

    printf( "Stack Overflow Hook called\n" );

    for( ; ; )
    {
    }
}
/*-----------------------------------------------------------*/
void vApplicationMallocFailedHook( void )
{
    /* If configUSE_MALLOC_FAILED_HOOK is set to 1 then this function will
     *  be called automatically if a call to pvPortMalloc() fails.  pvPortMalloc()
     *  is called automatically when a task, queue or semaphore is created. */
    printf( "Application Malloc Failed Hook called\n" );

    for( ; ; )
    {
    }
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer,
                                    StackType_t ** ppxIdleTaskStackBuffer,
                                    uint32_t * pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
     * state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer,
                                     StackType_t ** ppxTimerTaskStackBuffer,
                                     uint32_t * pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
 * function then they must be declared static - otherwise they will be allocated on
 * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
/*-----------------------------------------------------------*/