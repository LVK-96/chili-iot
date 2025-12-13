# Style checks
find_program(CLANG_FORMAT clang-format)


# Collect source files (excluding build directories and external libs)
# We want src/ and tests/ (excluding mocks perhaps if ignored in Makefile? Makefile said find src tests)
# Makefile: ALL_SRC_FILES := $(shell find src tests -name '*.c' -o -name '*.cpp' -o -name '*.h' -o -name '*.hpp')
file(GLOB_RECURSE ALL_STYLE_SRCS
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_SOURCE_DIR}/tests/*.c"
    "${CMAKE_SOURCE_DIR}/tests/*.cpp"
    "${CMAKE_SOURCE_DIR}/tests/*.h"
    "${CMAKE_SOURCE_DIR}/tests/*.hpp"
)

# Filter out build directory if it happens to be inside source (usually not if out-of-source, but good practice)
# Also filter out FetchContent deps if searched? No, GLOB_RECURSE only looks in specified paths. 
# src/ and tests/ are safe.

if(CLANG_FORMAT AND ALL_STYLE_SRCS)
    add_custom_target(style-check
        COMMAND ${CLANG_FORMAT} --Werror --dry-run ${ALL_STYLE_SRCS}
        COMMENT "Checking style with clang-format..."
    )

    add_custom_target(style-fix
        COMMAND ${CLANG_FORMAT} --Werror -i ${ALL_STYLE_SRCS}
        COMMENT "Fixing style with clang-format..."
    )
else()
    message(STATUS "clang-format not found. 'style-check' and 'style-fix' targets will be disabled.")
endif()


