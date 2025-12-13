# libopencm3 wrapper
# Since libopencm3 has a complex Makefile build, we use ExternalProject logic or custom commands.
# Here we use a Custom Target that runs 'make' in the fetched directory.

set(LIBOPENCM3_DIR ${libopencm3_SOURCE_DIR})
set(LIBOPENCM3_LIB ${LIBOPENCM3_DIR}/lib/libopencm3_stm32f1.a)

find_program(MAKE_EXE make REQUIRED)
add_custom_command(
    OUTPUT ${LIBOPENCM3_LIB}
    COMMAND ${MAKE_EXE} -j TARGETS=stm32/f1
    WORKING_DIRECTORY ${LIBOPENCM3_DIR}
    COMMENT "Building libopencm3 for STM32F1"
)

add_custom_target(libopencm3_build DEPENDS ${LIBOPENCM3_LIB})

add_library(libopencm3::libopencm3 STATIC IMPORTED GLOBAL)
add_dependencies(libopencm3::libopencm3 libopencm3_build)
set_target_properties(libopencm3::libopencm3 PROPERTIES
    IMPORTED_LOCATION ${LIBOPENCM3_LIB}
    INTERFACE_INCLUDE_DIRECTORIES "${LIBOPENCM3_DIR}/include"
)
