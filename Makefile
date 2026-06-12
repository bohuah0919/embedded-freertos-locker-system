CC                     =    arm-none-eabi-gcc
SIZE                   =    arm-none-eabi-size
BIN                   :=    RTOSDemo.axf

BUILD_DIR             :=    build

FREERTOS_DIR_REL      :=    ./FreeRTOS
FREERTOS_DIR          :=    $(abspath $(FREERTOS_DIR_REL))
KERNEL_DIR            :=    $(FREERTOS_DIR)

# Startup sources
SOURCE_FILES          +=    init/startup.c  syscall.c main.c

# platform portable source file
SOURCE_FILES          +=    $(KERNEL_DIR)/portable/GCC/ARM_CM3_MPU/port.c
SOURCE_FILES          +=    $(KERNEL_DIR)/portable/GCC/ARM_CM3_MPU/mpu_wrappers_v2_asm.c

# Kernel source files
SOURCE_FILES          +=    $(KERNEL_DIR)/portable/Common/mpu_wrappers.c
SOURCE_FILES          +=    $(KERNEL_DIR)/portable/Common/mpu_wrappers_v2.c
SOURCE_FILES          +=    $(KERNEL_DIR)/tasks.c
SOURCE_FILES          +=    $(KERNEL_DIR)/list.c
SOURCE_FILES          +=    $(KERNEL_DIR)/queue.c
SOURCE_FILES          +=    $(KERNEL_DIR)/timers.c
SOURCE_FILES          +=    $(KERNEL_DIR)/event_groups.c
SOURCE_FILES          +=    ${KERNEL_DIR}/portable/MemMang/heap_4.c
SOURCE_FILES          +=    $(KERNEL_DIR)/stream_buffer.c

# application source files
SOURCE_FILES          +=    app/app_main.c

# core system
SOURCE_FILES          +=    core/locker.c
SOURCE_FILES          +=    core/system_state.c

# event system
SOURCE_FILES          +=    event/event.c

# tasks
SOURCE_FILES          +=    tasks/delivery_task.c
SOURCE_FILES          +=    tasks/pickup_task.c
SOURCE_FILES          +=    tasks/logger_task.c
SOURCE_FILES          +=    tasks/auto_lock.c
SOURCE_FILES          +=    tasks/cli_task.c

# drivers
SOURCE_FILES          +=    drivers/uart_drv.c

INCLUDE_DIRS          +=    -I.
INCLUDE_DIRS          +=    -I./CMSIS
INCLUDE_DIRS          +=    -Icore
INCLUDE_DIRS          +=    -Ievent
INCLUDE_DIRS          +=    -Itasks
INCLUDE_DIRS          +=    -Iapp
INCLUDE_DIRS          +=    -Idrivers
INCLUDE_DIRS          +=    -I$(KERNEL_DIR)/include
INCLUDE_DIRS          +=    -I$(KERNEL_DIR)/portable/GCC/ARM_CM3_MPU

CPPFLAGS              +=    -DHEAP4

ifeq ($(PICOLIBC), 1)
    CFLAGS            +=     --specs=picolibc.specs -DPICOLIBC_INTEGER_PRINTF_SCANF
endif
CFLAGS                +=    -mthumb -mcpu=cortex-m3
ifeq ($(DEBUG), 1)
    CFLAGS            +=     -g3 -Og -ffunction-sections -fdata-sections -save-temps=obj
else
    CFLAGS            +=     -Os -ffunction-sections -fdata-sections
endif
#CFLAGS                +=    -flto
CFLAGS                +=    -Wall -Wextra -Wshadow -Wno-unused-parameter
CFLAGS                +=    $(INCLUDE_DIRS)

LDFLAGS                =    -T ./scripts/mps2_m3.ld
LDFLAGS               +=    -Xlinker -Map=${BUILD_DIR}/output.map
LDFLAGS               +=    -Xlinker --gc-sections
#LDFLAGS               +=    -Xlinker --print-gc-sections
#LDFLAGS               +=    -nostartfiles -nostdlib -nolibc -nodefaultlibs
LDFLAGS               +=    -nostdlib



OBJ_FILES             :=    $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.o)

.PHONY: clean

$(BUILD_DIR)/$(BIN) : $(OBJ_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $+ -o $(@)
	$(SIZE) $(@)

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

INCLUDES              :=     $(SOURCE_FILES:%.c=$(BUILD_DIR)/%.d)
-include $(INCLUDES)

${BUILD_DIR}/%.o : %.c Makefile
	-mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -c $< -o $@

clean:
	-rm -rf build
