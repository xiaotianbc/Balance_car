##
## Created by xiaotian on 2022/9/25.
##

##cmake 编译配置文件
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.11)

# specify cross-compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# project settings
project(f401_balance_car_bootloader C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

#添加硬件浮点数配置
add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

#Uncomment for software floating point
#add_compile_options(-mfloat-abi=soft)

add_compile_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

# uncomment to mitigate c++17 absolute addresses warnings
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

# Enable assembler files preprocessing
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

#设置编译器优化等级
set(Build_str g)

if ("${Build_str}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${Build_str}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${Build_str}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif ()

add_definitions(-DCONFIG_USB_DWC2_PORT=FS_PORT)


#预处理器包含的文件夹
include_directories(
        STM32F4xx_StdPeriph_Driver/CMSIS
        STM32F4xx_StdPeriph_Driver/inc
        User
        board_dev
        board_dev/lcd
        mcu_dev
        middleware/ringbuffer

        #        /*cmsis rtos2*/
        middleware/FreeRTOS/Source/include
        middleware/FreeRTOS/Source/portable/GCC/ARM_CM4F
        # segger rtt
        middleware/SEGGER_RTT_V782b/Config
        middleware/SEGGER_RTT_V782b/RTT
        middleware/common_tool

        middleware/smart_printf/

        middleware/lvgl-8.3.3
        middleware/littlefs

        middleware/sfud/inc
)


add_definitions(-DUSE_STDPERIPH_DRIVER -DSTM32F401xx)

##需要参与编译的文件
file(GLOB_RECURSE SOURCES
        User/*.*
        STM32F4xx_StdPeriph_Driver/src/*.*
        STM32F4xx_StdPeriph_Driver/CMSIS/startup_stm32f401xx.s
        board_dev/*.*
        mcu_dev/*.*
        middleware/ringbuffer/*.*
        # CMSIS-RTOS2
        middleware/FreeRTOS/Source/croutine.c
        middleware/FreeRTOS/Source/event_groups.c
        middleware/FreeRTOS/Source/list.c
        middleware/FreeRTOS/Source/queue.c
        middleware/FreeRTOS/Source/stream_buffer.c
        middleware/FreeRTOS/Source/tasks.c
        middleware/FreeRTOS/Source/timers.c
        middleware/FreeRTOS/Source/portable/MemMang/heap_4.c
        middleware/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c


        # segger rtt
        middleware/SEGGER_RTT_V782b/RTT/*.*
        middleware/SEGGER_RTT_V782b/Syscalls/SEGGER_RTT_Syscalls_GCC.c

        middleware/common_tool/*.c

        middleware/smart_printf/printf.c

        middleware/sfud/*.c

        middleware/lvgl-8.3.3/*.c

        middleware/littlefs/*.c
        )

#排除stm32F401不支持的fsmc 和fmc
file(GLOB_RECURSE EXCLUDE_SRCS
        STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.c
        STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fsmc.c

        )
# 从源文件列表(SOURCES)中剔除干扰文件(EXCLUDE_SRCS)
list(REMOVE_ITEM SOURCES ${EXCLUDE_SRCS})

#设置链接脚本
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/GCC_Port/STM32F401CEUX_FLASH.ld)

add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
#add_link_options(--specs=nosys.specs)
add_link_options(-mcpu=cortex-m4 -mthumb -mthumb-interwork)
add_link_options(-T ${LINKER_SCRIPT})

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")
