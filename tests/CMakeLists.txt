# =============================================================================
# Tests CMakeLists.txt
# =============================================================================
# Parent has already called FetchContent_MakeAvailable for dependencies
#
# Mock Architecture:
# -----------------
# mocks/
# ├── mock_libopencm3.cpp  - Mocks for libopencm3 functions (GPIO, USART, I2C, DMA, etc.)
# ├── mock_libopencm3.h    - Headers for mock state access
# ├── mock_interrupts.cpp  - Stub interrupt handlers for tests
# ├── test_events.cpp      - Test event recording system for assertions
# ├── test_events.h        - Test event API
# ├── MockRTOS.h           - C++ mock FreeRTOS interface
# ├── MockTemperatureSensor.h - C++ mock sensor
# └── stubs/               - Header stubs for host builds (no ARM toolchain)
#     ├── freertos/        - FreeRTOS header stubs + mock implementation
#     └── hal/             - libopencm3 header stubs
#
# QEMU Targets:
# - runner:              Unit tests with mocked FreeRTOS + mocked hardware
# - firmware-qemu-debug: Firmware sim with real FreeRTOS + mocked hardware

# =============================================================================
# Common source definitions (shared between targets)
# =============================================================================

# Project source files (excluding entry points and HW-specific code)
file(GLOB_RECURSE COMMON_PROJ_SRCS "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(FILTER COMMON_PROJ_SRCS EXCLUDE REGEX ".*main\\.cpp$")
list(FILTER COMMON_PROJ_SRCS EXCLUDE REGEX ".*interrupts\\.cpp$")
list(FILTER COMMON_PROJ_SRCS EXCLUDE REGEX ".*syscalls\\.cpp$")

# Hardware mock implementations
set(HARDWARE_MOCKS
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/mock_libopencm3.cpp
)

# Test infrastructure
set(TEST_INFRA
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/test_events.cpp
)

# FreeRTOS mock (for unit tests only)
set(FREERTOS_MOCK
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/stubs/freertos/mock_freertos.cpp
)

# Interrupt mock (for unit tests only)
set(INTERRUPT_MOCK
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks/mock_interrupts.cpp
)

# Common include directories
set(COMMON_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# =============================================================================
# ARM Cross-Compile (QEMU Targets)
# =============================================================================
if(CMAKE_SYSTEM_NAME STREQUAL "Generic")

    # Remove specs that conflict with semihosting
    string(REPLACE "--specs=nano.specs" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
    string(REPLACE "--specs=nosys.specs" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")

    # -------------------------------------------------------------------------
    # runner - Unit tests on QEMU (mocked FreeRTOS + mocked hardware)
    # -------------------------------------------------------------------------
    file(GLOB_RECURSE TEST_SRCS "test_*.cpp")

    add_executable(runner
        ${CMAKE_CURRENT_SOURCE_DIR}/main_qemu.cpp
        ${CMAKE_SOURCE_DIR}/src/syscalls.cpp
        ${TEST_SRCS}
        ${COMMON_PROJ_SRCS}
        ${HARDWARE_MOCKS}
        ${TEST_INFRA}
        ${FREERTOS_MOCK}
        ${INTERRUPT_MOCK}
    )

    target_compile_features(runner PRIVATE cxx_std_23)
    target_compile_options(runner PRIVATE -fexceptions)
    target_compile_definitions(runner PRIVATE
        IS_TEST_BINARY
        SEMIHOSTING_ENV
        QEMU_ENV
        ARM_TEST=1
        DOCTEST_CONFIG_NO_MULTITHREADING
        DOCTEST_CONFIG_NO_POSIX_SIGNALS
    )

    target_include_directories(runner PRIVATE
        ${COMMON_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks/stubs/freertos/freertos
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks/stubs/freertos
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks/stubs/hal
        ${doctest_SOURCE_DIR}/doctest
    )

    target_link_libraries(runner PRIVATE
        doctest::doctest
        bme280
        libopencm3::libopencm3
    )

    target_link_options(runner PRIVATE
        -T${CMAKE_SOURCE_DIR}/qemu-mps2-an385.ld
        -Wl,--allow-multiple-definition
        --specs=rdimon.specs
    )

    # Generate .bin file
    add_custom_command(TARGET runner POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Obinary runner runner.bin
        COMMENT "Generating runner.bin"
    )

    # Run tests in QEMU
    add_custom_target(qemu-test
        COMMAND qemu-system-arm -M mps2-an385 -m 16M -nographic -semihosting -echr 24
                -kernel runner
        DEPENDS runner
        USES_TERMINAL
        COMMENT "Running unit tests in QEMU (mps2-an385)..."
    )

    enable_testing()
    include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)

    # -------------------------------------------------------------------------
    # firmware-qemu-debug - Firmware simulation (real FreeRTOS + mocked hardware)
    # -------------------------------------------------------------------------
    add_executable(firmware-qemu-debug
        ${CMAKE_SOURCE_DIR}/src/main.cpp
        ${CMAKE_SOURCE_DIR}/src/syscalls.cpp
        ${COMMON_PROJ_SRCS}
        ${HARDWARE_MOCKS}
        ${INTERRUPT_MOCK}
        # Note: NO FreeRTOS mock (uses real FreeRTOS), NO test infra
    )

    target_compile_features(firmware-qemu-debug PRIVATE cxx_std_23)
    target_compile_definitions(firmware-qemu-debug PRIVATE
        SEMIHOSTING_ENV
        QEMU_ENV
        IS_FIRMWARE_BINARY
    )

    target_include_directories(firmware-qemu-debug PRIVATE
        ${COMMON_INCLUDE_DIRS}
    )

    target_link_libraries(firmware-qemu-debug PRIVATE
        bme280
        libopencm3::libopencm3
        freertos_kernel
        freertos_config
    )

    target_link_options(firmware-qemu-debug PRIVATE
        -T${CMAKE_SOURCE_DIR}/qemu-mps2-an385.ld
        --specs=rdimon.specs
        -Wl,--start-group
        -lc
        -lrdimon
        -Wl,--end-group
    )

    # Generate .bin file
    add_custom_command(TARGET firmware-qemu-debug POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Obinary firmware-qemu-debug firmware-qemu-debug.bin
        COMMENT "Generating firmware-qemu-debug.bin"
    )

    # Run firmware in QEMU
    add_custom_target(qemu-sim
        COMMAND qemu-system-arm -M mps2-an385 -m 16M -nographic -semihosting -echr 24
                -kernel firmware-qemu-debug
        DEPENDS firmware-qemu-debug
        USES_TERMINAL
        COMMENT "Running firmware simulation in QEMU (mps2-an385)..."
    )

# =============================================================================
# Host Native Build (x86 Tests)
# =============================================================================
else()

    file(GLOB_RECURSE TEST_SRCS "test_*.cpp")

    add_executable(runner
        ${CMAKE_CURRENT_SOURCE_DIR}/main_host.cpp
        ${TEST_SRCS}
        ${COMMON_PROJ_SRCS}
        ${HARDWARE_MOCKS}
        ${TEST_INFRA}
        ${FREERTOS_MOCK}
        ${INTERRUPT_MOCK}
    )

    target_compile_features(runner PRIVATE cxx_std_23)
    target_compile_definitions(runner PRIVATE
        IS_TEST_BINARY
        HOST_ENV
    )

    target_include_directories(runner PRIVATE
        ${COMMON_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks/stubs/freertos/freertos
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks/stubs/freertos
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks/stubs/hal
        ${doctest_SOURCE_DIR}/doctest
    )

    target_link_libraries(runner PRIVATE
        doctest::doctest
        bme280
    )

    enable_testing()
    include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
    doctest_discover_tests(runner)

    add_custom_target(test
        COMMAND $<TARGET_FILE:runner>
        DEPENDS runner
        USES_TERMINAL
    )

endif()
