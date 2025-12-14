# --- Dependencies (Declarations) ---
include(cmake/dependencies.cmake)

# --- FreeRTOS Configuration ---
# We defined the sources manually below to match legacy Makefile exactly
# (Avoiding standard CMake 'heap_4.c' and 'croutine.c')

add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE src)

# --- FetchContent Population ---
# Manually populate FreeRTOS to control its build
cmake_policy(SET CMP0169 OLD)
FetchContent_GetProperties(FreeRTOS-Kernel)
if(NOT freertos-kernel_POPULATED)
    FetchContent_Populate(FreeRTOS-Kernel)
endif()

# Populate others
FetchContent_MakeAvailable(libopencm3 BME280_driver)

include(cmake/libopencm3.cmake)
include(cmake/bme280.cmake)

# --- Manual FreeRTOS Build ---
# Legacy Makefile included: event_groups.c, list.c, queue.c, stream_buffer.c, tasks.c, timers.c
# AND src/heap_useNewlib_bluepill.c
# AND port.c

set(FREERTOS_SRC
    "${freertos-kernel_SOURCE_DIR}/event_groups.c"
    "${freertos-kernel_SOURCE_DIR}/list.c"
    "${freertos-kernel_SOURCE_DIR}/queue.c"
    "${freertos-kernel_SOURCE_DIR}/stream_buffer.c"
    "${freertos-kernel_SOURCE_DIR}/tasks.c"
    "${freertos-kernel_SOURCE_DIR}/timers.c"
    "${freertos-kernel_SOURCE_DIR}/portable/GCC/ARM_CM3/port.c"
    "src/heap_useNewlib_bluepill.c"
)

add_library(freertos_kernel STATIC ${FREERTOS_SRC})

target_include_directories(freertos_kernel PUBLIC
    "${freertos-kernel_SOURCE_DIR}/include"
    "${freertos-kernel_SOURCE_DIR}/portable/GCC/ARM_CM3"
)

target_link_libraries(freertos_kernel PUBLIC freertos_config)

# Suppress warnings in FreeRTOS if needed (legacy had -Wno-unused-variable for port.c)
set_source_files_properties("${freertos-kernel_SOURCE_DIR}/portable/GCC/ARM_CM3/port.c"
    PROPERTIES COMPILE_OPTIONS "-Wno-unused-variable")
