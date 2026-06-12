#ifndef CLI_TASK_H
#define CLI_TASK_H

#define CLI_STACK_SIZE_WORDS    1024
#define CLI_STACK_SIZE_BYTES    (CLI_STACK_SIZE_WORDS * 4)

void cli_task(void *arg);

#endif /* CLI_TASK_H */