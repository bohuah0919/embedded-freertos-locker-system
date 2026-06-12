#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "event.h"
#include "uart_drv.h"
#include "cli_task.h"

StackType_t cli_stack[CLI_STACK_SIZE_WORDS] 
    __attribute__((aligned(CLI_STACK_SIZE_BYTES))) 
    __attribute__((section(".unprivileged_data")));

StaticTask_t cli_tcb 
    __attribute__((aligned(32))) 
    __attribute__((section(".unprivileged_data")));

void cli_task(void *arg)
{
    (void)arg;
    char command_line[UART_CMD_MAX_LEN];
    static int parcel_counter = 1;

    /* Initialize the underlying physical UART hardware */
    uart_init();

    printf("\n======================================================\n");
    printf("   INTERACTIVE SMART-LOCKER CLI ACTIVE                \n");
    printf("   Commands:                                          \n");
    printf("     'd'          -> Trigger Next Sequential Delivery  \n");
    printf("     'p[ID]_[PIN]'-> Pickup (ex: p001_1000 or p002_1001)\n");
    printf("======================================================\n\n");

    while (1)
    {
        /* Poll the driver for a complete command line (\r or \n received) */
        if (uart_poll_command(command_line, UART_CMD_MAX_LEN) == pdTRUE)
        {
            printf("\n[CLI Parsing] Evaluating: \"%s\"\n", command_line);

            /* -----------------------------------------------------------------
             * 1. CASE: Courier Delivery ('d')
             * ----------------------------------------------------------------- */
            if (command_line[0] == 'd' && command_line[1] == '\0')
            {
                event_t evt;
                memset(&evt, 0, sizeof(event_t));
                evt.type = EVT_DELIVERY_REQUEST;
                snprintf(evt.delivery.parcel_id, PARCEL_ID_LEN, "PKG%03d", parcel_counter);

                printf("[CLI] Dispatching Delivery Request: %s\n", evt.delivery.parcel_id);
                event_publish(&evt);
                parcel_counter++;
            }
            /* -----------------------------------------------------------------
             * 2. CASE: Customer Multi-Auth Pickup ('p[ID]_[PIN]')
             * ----------------------------------------------------------------- */
            else if (command_line[0] == 'p' && strlen(command_line) > 2)
            {
                char *underscore = strchr(command_line, '_');
                if (underscore != NULL)
                {
                    *underscore = '\0';
                    int parsed_id_num = atoi(&command_line[1]);
                    int parsed_pin    = atoi(underscore + 1);

                    event_t evt;
                    memset(&evt, 0, sizeof(event_t));
                    evt.type = EVT_PICKUP_REQUEST;
                    snprintf(evt.pickup.parcel_id, PARCEL_ID_LEN, "PKG%03d", parsed_id_num);
                    evt.pickup.pin = parsed_pin;

                    printf("[CLI] Authenticating Pickup -> Target: %s, PIN: %d\n", 
                           evt.pickup.parcel_id, evt.pickup.pin);
                    event_publish(&evt);
                }
                else
                {
                    printf("[CLI ERROR] Invalid pickup syntax. Use: p[ID]_[PIN]\n");
                }
            }
            else
            {
                printf("[CLI ERROR] Command unmapped. Use 'd' or 'p[ID]_[PIN]'\n");
            }
        }

        /* Keep polling responsive and friendly to the scheduler */
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}