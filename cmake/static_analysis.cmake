# Static Analysis
find_program(CLANG_TIDY clang-tidy)

# Re-collect source files to be independent (or could share variable if included after style.cmake, but cleaner to be self-contained)
file(GLOB_RECURSE ALL_ANALYSIS_SRCS
    "${CMAKE_SOURCE_DIR}/src/*.c"
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/tests/*.c"
    "${CMAKE_SOURCE_DIR}/tests/*.cpp"
)

# Note: clang-tidy usually runs on C/C++ sources, not headers directly (headers are checked when included)
  
if(CLANG_TIDY AND ALL_ANALYSIS_SRCS)
    add_custom_target(clang-tidy
        COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} ${ALL_ANALYSIS_SRCS}
        COMMENT "Running clang-tidy..."
    )

    add_custom_target(clang-tidy-fix
        COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} --fix ${ALL_ANALYSIS_SRCS}
        COMMENT "Fixing issues with clang-tidy..."
    )
else()
    message(STATUS "clang-tidy not found. 'clang-tidy' targets will be disabled.")
endif()
