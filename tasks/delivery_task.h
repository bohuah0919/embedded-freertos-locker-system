#ifndef DELIVERY_TASK_H
#define DELIVERY_TASK_H

#define DELIVERY_STACK_SIZE_WORDS  1024
#define DELIVERY_STACK_SIZE_BYTES  (DELIVERY_STACK_SIZE_WORDS * 4)

void delivery_task_init(void);
void delivery_task(void *arg);

#endif