#ifndef UART_DRV_H
#define UART_DRV_H

#include <stdint.h>
#include "FreeRTOS.h"

#define UART_CMD_MAX_LEN  32

typedef struct UART_t
{
    volatile uint32_t DATA;
    volatile uint32_t STATE;
    volatile uint32_t CTRL;
    volatile uint32_t INTSTATUS;
    volatile uint32_t BAUDDIV;
} UART_t;

#define UART0_ADDR           ( ( UART_t * ) ( 0x40004000 ) )
#define UART_DR( baseaddr )  ( *( unsigned int * ) ( baseaddr ) )

/* Register Bit Definitions matching the official hardware specifications */
#define UART_STATE_TXFULL    ( 1 << 0 )
#define UART_STATE_RXFULL    ( 1 << 1 ) /* Explicitly added for reading state */

#define UART_CTRL_TX_EN      ( 1 << 0 )
#define UART_CTRL_RX_EN      ( 1 << 1 )

/* Exported clean polling API */
void uart_init(void);
BaseType_t uart_poll_command(char *out_cmd_buf, size_t max_len);

#endif /* UART_DRV_H */