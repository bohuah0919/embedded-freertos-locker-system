#include "app_main.h"
#include <FreeRTOS.h>
#include <task.h>

#include <FreeRTOSConfig.h>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

int main( void )
{
    app_main();
    return 0;
}

/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
}
/*-----------------------------------------------------------*/

void vAssertCalled( void )
{
    volatile unsigned long looping = 0;

    taskENTER_CRITICAL();
    {
        /* Use the debugger to set ul to a non-zero value in order to step out
         *      of this function to determine why it was called. */
        while( looping == 0LU )
        {
            portNOP();
        }
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/
void vLoggingPrintf( const char * pcFormat,
                     ... )
{
    va_list arg;

    va_start( arg, pcFormat );
    vprintf( pcFormat, arg );
    va_end( arg );
}