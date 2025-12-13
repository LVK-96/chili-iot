cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(chili-iot C CXX ASM)
add_compile_definitions(STM32F1)

# Determine build mode based on CMAKE_SYSTEM_NAME
# 'Generic' = Firmware (cross-compile), 'Linux' = Host (tests)
if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    # Cross build -> Default to Firmware
    set(CHILI_BUILD_TESTS OFF)
else()
     # Host build -> Default to Tests
    set(CHILI_BUILD_TESTS ON)
endif()


# --- Configuration & Secrets ---
include(cmake/setup_secrets.cmake)
include(cmake/dev_helpers.cmake)

# --- Dependencies ---
if(NOT CHILI_BUILD_TESTS)
    include(cmake/freertos.cmake)

    # --- Main Firmware ---

    # Collect source files
    file(GLOB_RECURSE SRC_FILES "src/*.cpp")

    add_executable(sensor_node ${SRC_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/stm32f103c8t6.ld)

    # Compile options for C++23
    target_compile_features(sensor_node PRIVATE cxx_std_23)

    # Include directories
    target_include_directories(sensor_node PRIVATE 
        src
    )

    # Link libraries
    target_link_libraries(sensor_node PRIVATE
        libopencm3::libopencm3
        freertos_kernel
        freertos_config # Our config interface
        bme280
    )

    # Linker script
    target_link_options(sensor_node PRIVATE
        -T${CMAKE_CURRENT_SOURCE_DIR}/stm32f103c8t6.ld
        -Wl,-Map=${CMAKE_BINARY_DIR}/sensor_node.map
        # FreeRTOS + Newlib needs some care with syscalls
    )
    
    # Inject secrets via -include to hide from process list
    target_compile_options(sensor_node PRIVATE ${SECRETS_COMPILE_OPTIONS})

    # Create .bin file
    add_custom_command(TARGET sensor_node POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Obinary sensor_node sensor_node.bin
        COMMENT "Generating binary file sensor_node.bin"
    )

    # Print size
    add_custom_command(TARGET sensor_node POST_BUILD
        COMMAND ${CMAKE_SIZE} sensor_node
        COMMENT "Firmware size:"
    )

    # --- Debug Targets ---
    
    # Flash target
    find_program(STFLASH st-flash)
    if(STFLASH)
        add_custom_target(flash
            COMMAND ${STFLASH} --connect-under-reset write ${CMAKE_BINARY_DIR}/sensor_node.bin 0x8000000
            DEPENDS sensor_node
            COMMENT "Flashing sensor_node.bin to device..."
        )
    else()
        message(STATUS "st-flash not found. 'flash' target will be disabled.")
    endif()

    # Disassemble target
    add_custom_target(disassemble
        COMMAND ${CMAKE_OBJDUMP} --wide --syms --source --line-numbers sensor_node > sensor_node.dump
        DEPENDS sensor_node
        COMMENT "Disassembling sensor_node to sensor_node.dump..."
    )

    # GDB Server target
    find_program(STUTIL st-util)
    if(STUTIL)
        add_custom_target(gdb-server
            COMMAND ${STUTIL}
            COMMENT "Starting GDB server..."
            USES_TERMINAL
        )
    else()
        message(STATUS "st-util not found. 'gdb-server' target will be disabled.")
    endif()



else()
    # Tests rely on some dependencies too, like FetchContent declarations
    # But specifically NOT the main firmware build or FreeRTOS Kernel build for ARM
    # We still need dependencies.cmake
    include(cmake/dependencies.cmake)
    
    # Suppress warnings for legacy dependencies (doctest 2.4.11 triggers CMake < 3.5 warning)
    set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(libopencm3 BME280_driver doctest)
    set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)
    
    include(cmake/bme280.cmake) # Used by tests
endif()

# --- Code Quality (Universal) ---
include(cmake/style.cmake)
include(cmake/static_analysis.cmake)

# --- Tests ---
option(CHILI_BUILD_TESTS "Build native tests" OFF)
if(CHILI_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# --- MQTT Broker (Universal) ---
if(EXISTS "${CMAKE_SOURCE_DIR}/.venv/bin/python3")
    set(VENV_PYTHON "${CMAKE_SOURCE_DIR}/.venv/bin/python3")
    add_custom_target(mqtt-broker
        COMMAND ${VENV_PYTHON} ${CMAKE_SOURCE_DIR}/scripts/esp8266_test/run_broker.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Starting MQTT test broker on port 1883 (using venv)..."
        USES_TERMINAL
    )
else()
    find_program(PYTHON3 python3)
    if(PYTHON3)
        add_custom_target(mqtt-broker
            COMMAND ${PYTHON3} ${CMAKE_SOURCE_DIR}/scripts/esp8266_test/run_broker.py
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Starting MQTT test broker on port 1883..."
            USES_TERMINAL
        )
        message(WARNING "Virtual environment not found at ${CMAKE_SOURCE_DIR}/.venv. Using system python3. Run 'source setup_env' first if broker fails.")
    else()
        message(STATUS "python3 not found. 'mqtt-broker' target will be disabled.")
    endif()
endif()



