#include "uart_drv.h"
#include <string.h>
#include <stdio.h>

/* Hidden Internal Drivers State (Protected from other files via static) */
static char internal_rx_buffer[UART_CMD_MAX_LEN] __attribute__((section(".unprivileged_data")));
static int internal_buf_idx __attribute__((section(".unprivileged_data"))) = 0;

void uart_init(void)
{
    UART0_ADDR->BAUDDIV = 16;
    
    /* Safely enables both channels globally for printf and CLI */
    UART0_ADDR->CTRL = (UART_CTRL_TX_EN | UART_CTRL_RX_EN);
    
    UART0_ADDR->STATE = 0;
    internal_buf_idx = 0;
    memset(internal_rx_buffer, 0, UART_CMD_MAX_LEN);
}

BaseType_t uart_poll_command(char *out_cmd_buf, size_t max_len)
{
    if (out_cmd_buf == NULL || max_len == 0)
    {
        return pdFALSE;
    }

    /* Check if hardware rx state register reports a character is waiting */
    if (UART0_ADDR->STATE & UART_STATE_RXFULL)
    {
        /* Pull character off the hardware structural data window */
        char c = (char)(UART0_ADDR->DATA & 0xFF);

        if (c == '\0')
        {
            return pdFALSE;
        }

        /* Echo typed input back immediately to show visible interactive output */
        putchar(c);
        fflush(stdout);

        /* Clean evaluation of line carriage returns */
        if (c == '\r' || c == '\n')
        {
            internal_rx_buffer[internal_buf_idx] = '\0';

            if (internal_buf_idx > 0)
            {
                strncpy(out_cmd_buf, internal_rx_buffer, max_len - 1);
                out_cmd_buf[max_len - 1] = '\0';

                /* Flush buffer states back to zero tracking */
                internal_buf_idx = 0;
                memset(internal_rx_buffer, 0, UART_CMD_MAX_LEN);
                
                return pdTRUE; 
            }
            internal_buf_idx = 0;
        }
        else
        {
            if (internal_buf_idx < (UART_CMD_MAX_LEN - 1))
            {
                internal_rx_buffer[internal_buf_idx++] = c;
            }
        }
    }

    return pdFALSE;
}