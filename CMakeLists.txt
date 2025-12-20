cmake_minimum_required(VERSION 3.20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM "3.5")

project(chili-iot C CXX ASM)
add_compile_definitions(STM32F1)

# Determine build mode based on CMAKE_SYSTEM_NAME
# 'Generic' = ARM cross-compile, 'Linux/Darwin/Windows' = Host native
message(STATUS "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}")

# --- Configuration & Secrets ---
include(cmake/setup_secrets.cmake)
include(cmake/dev_helpers.cmake)

# --- Dependencies ---
include(cmake/dependencies.cmake)

# Suppress warnings for legacy dependencies
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    # ARM: Include FreeRTOS (which also includes bme280.cmake)
    include(cmake/freertos.cmake)
    FetchContent_MakeAvailable(libopencm3 BME280_driver doctest)
else()
    # Host: Only need test dependencies
    FetchContent_MakeAvailable(libopencm3 BME280_driver doctest)
    include(cmake/bme280.cmake)
endif()
set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)

# =============================================================================
# ARM Cross-Compile Targets
# =============================================================================
if(CMAKE_SYSTEM_NAME STREQUAL "Generic")
    
    # Common source files for firmware
    file(GLOB_RECURSE FIRMWARE_SRCS "src/*.cpp")
    
    # -------------------------------------------------------------------------
    # firmware - Production build (no semihosting)
    # -------------------------------------------------------------------------
    add_executable(firmware ${FIRMWARE_SRCS})
    
    target_compile_features(firmware PRIVATE cxx_std_23)
    target_include_directories(firmware PRIVATE src)
    target_link_libraries(firmware PRIVATE
        libopencm3::libopencm3
        freertos_kernel
        freertos_config
        bme280
    )
    target_link_options(firmware PRIVATE
        -T${CMAKE_SOURCE_DIR}/stm32f103c8t6.ld
        -Wl,-Map=${CMAKE_BINARY_DIR}/firmware.map
    )
    target_compile_options(firmware PRIVATE ${SECRETS_COMPILE_OPTIONS})
    target_compile_definitions(firmware PRIVATE
        IS_FIRMWARE_BINARY
    )

    # Post-build: create .bin and print size
    add_custom_command(TARGET firmware POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Obinary firmware firmware.bin
        COMMAND ${CMAKE_SIZE} firmware
        COMMENT "Generating firmware.bin"
    )

    # -------------------------------------------------------------------------
    # firmware-debug - Semihosting build (for use with debugger/SWD)
    # -------------------------------------------------------------------------
    add_executable(firmware-debug ${FIRMWARE_SRCS})
    
    target_compile_features(firmware-debug PRIVATE cxx_std_23)
    target_include_directories(firmware-debug PRIVATE src)
    target_link_libraries(firmware-debug PRIVATE
        libopencm3::libopencm3
        freertos_kernel
        freertos_config
        bme280
    )
    # Remove nano.specs and nosys.specs for semihosting (they conflict with rdimon.specs)
    string(REPLACE "--specs=nano.specs" "" DEBUG_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
    string(REPLACE "--specs=nosys.specs" "" DEBUG_LINKER_FLAGS "${DEBUG_LINKER_FLAGS}")
    set_target_properties(firmware-debug PROPERTIES LINK_FLAGS "${DEBUG_LINKER_FLAGS}")
    target_link_options(firmware-debug PRIVATE
        -T${CMAKE_SOURCE_DIR}/stm32f103c8t6.ld
        -Wl,-Map=${CMAKE_BINARY_DIR}/firmware-debug.map
        --specs=rdimon.specs
    )
    target_compile_options(firmware-debug PRIVATE ${SECRETS_COMPILE_OPTIONS})
    target_compile_definitions(firmware-debug PRIVATE
        IS_FIRMWARE_BINARY
        SEMIHOSTING_ENV
    )

    add_custom_command(TARGET firmware-debug POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Obinary firmware-debug firmware-debug.bin
        COMMAND ${CMAKE_SIZE} firmware-debug
        COMMENT "Generating firmware-debug.bin (semihosting enabled)"
    )

    # -------------------------------------------------------------------------
    # Tests subdirectory (QEMU targets: runner, qemu-app)
    # -------------------------------------------------------------------------
    add_subdirectory(tests)

    # -------------------------------------------------------------------------
    # Utility targets
    # -------------------------------------------------------------------------
    add_custom_target(disassemble
        COMMAND ${CMAKE_OBJDUMP} --wide --syms --source --line-numbers firmware > firmware.dump
        DEPENDS firmware
        COMMENT "Disassembling firmware..."
    )

    find_program(STUTIL st-util)
    if(STUTIL)
        add_custom_target(gdb-server
            COMMAND ${STUTIL}
            COMMENT "Starting GDB server..."
            USES_TERMINAL
        )
    endif()

    add_custom_target(flash
        COMMAND ${CMAKE_SOURCE_DIR}/flash.sh ${CMAKE_BINARY_DIR}/firmware.bin
        DEPENDS firmware
        COMMENT "Flashing firmware.bin to device..."
        USES_TERMINAL
    )
    add_custom_target(flash-debug
        COMMAND ${CMAKE_SOURCE_DIR}/flash.sh ${CMAKE_BINARY_DIR}/firmware-debug.bin
        DEPENDS firmware-debug
        COMMENT "Flashing firmware-debug.bin (semihosting) to device..."
        USES_TERMINAL
    )

# =============================================================================
# Host Native Targets (x86)
# =============================================================================
else()
    # Host builds only have tests
    add_subdirectory(tests)
endif()

# --- Code Quality (Universal) ---
include(cmake/style.cmake)
include(cmake/static_analysis.cmake)

# --- Help ---
include(cmake/chili_help.cmake)