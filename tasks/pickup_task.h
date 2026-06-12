#ifndef PICKUP_TASK_H
#define PICKUP_TASK_H

#define PICKUP_STACK_SIZE_WORDS    1024
#define PICKUP_STACK_SIZE_BYTES    (PICKUP_STACK_SIZE_WORDS * 4)

void pickup_task_init(void);
void pickup_task(void *arg);

#endif