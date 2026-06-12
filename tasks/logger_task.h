#ifndef LOGGER_TASK_H
#define LOGGER_TASK_H

#define LOGGER_STACK_SIZE_WORDS    1024
#define LOGGER_STACK_SIZE_BYTES    (LOGGER_STACK_SIZE_WORDS * 4)

void logger_task_init(void);
void logger_task(void *arg);

#endif