#THIS FILE IS AUTO GENERATED FROM THE TEMPLATE! DO NOT CHANGE!
set(CMAKE_SYSTEM_NAME JVOS_SDK)
set(CMAKE_SYSTEM_VERSION 0.00.1)
cmake_minimum_required(VERSION 3.16)


# specify cross-compilers and tools
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(USE_ROM_SIZE STM32F10X_MD)
set(OS_LIB jvos/component/os/freertos/freertos_23.4.10/Source)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# project settings
project(JVOS C CXX ASM)

#Uncomment for hardware floating point
#add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
#add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
#add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

#Uncomment for software floating point
#add_compile_options(-mfloat-abi=soft)

add_compile_options(-mcpu=cortex-m3 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

# uncomment to mitigate c++17 absolute addresses warnings
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-register")

# Enable assembler files preprocessing
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif ()

include_directories(jvos/component/soc/stm32/stm32f10x 
                    jvos/component/soc/stm32/stm32f10x/fwlib/include
                    jvos/impl/tartget/stm32f10x/include/mhal
                    jvos/impl/tartget/stm32f10x/include
                    jvos/component/os/freertos/freertos_23.4.10/Source/include
                    jvos/component/os/freertos/freertos_23.4.10/Source/portable/GCC/ARM_CM3
        )

add_definitions(-DDEBUG -DSTM32F103xE -DUSE_STDPERIPH_DRIVER -D${USE_ROM_SIZE})

file(GLOB_RECURSE SOURCES "app/main.c" 
                            "jvos/component/soc/stm32/stm32f10x/system_stm32f10x.c" 
                            "jvos/impl/tartget/stm32f10x/impl/*/*.c" 
                            "jvos/component/soc/stm32/stm32f10x/fwlib/*/*.c" 
                            "${OS_LIB}/portable/GCC/ARM_CM3/port.c" 
                            "${OS_LIB}/portable/MemMang/heap5.c" 
                            "${OS_LIB}/croutine.c"
                            "${OS_LIB}/event_groups.c"
                            "${OS_LIB}/list.c"
                            "${OS_LIB}/queue.c"
                            "${OS_LIB}/tasks.c"
                            "${OS_LIB}/timers.c"
                            "${OS_LIB}/stream_buffer.c"
                            )

if(USE_ROM_SIZE STREQUAL STM32F10X_HD)
    list(APPEND SOURCES "jvos/component/soc/stm32/stm32f10x/startup/startup_stm32f10x_md.s")
elseif(USE_ROM_SIZE STREQUAL STM32F10X_MD)
    list(APPEND SOURCES "jvos/component/soc/stm32/stm32f10x/startup/startup_stm32f10x_md.s")
endif()
message(STATUS >>> ${USE_ROM_SIZE})

list(APPEND SOURCES "")

set(LINKER_SCRIPT ../jvos/component/soc/stm32/stm32f10x/Link/Medium_density/stm32_flash.ld)

add_link_options(-specs=nano.specs -specs=nosys.specs -u _printf_float)
add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=cortex-m3 -mthumb -mthumb-interwork)
add_link_options(-T ${LINKER_SCRIPT})

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")