if(NOT CHILI_BUILD_TESTS)
    return()
endif()

# Find source files - we likely need to exclude main.cpp or hardware-specific things
# Reuse the source logic or list manually?
# For now, let's grab src/*.cpp but filter out main.cpp
file(GLOB_RECURSE PROJ_SRCS "${CMAKE_SOURCE_DIR}/src/*.cpp")
list(FILTER PROJ_SRCS EXCLUDE REGEX ".*main\\.cpp$")
list(FILTER PROJ_SRCS EXCLUDE REGEX ".*interrupts\\.cpp$") # Interrupts often hard to test natively

# Test sources
file(GLOB_RECURSE TEST_SRCS "*.cpp")
file(GLOB_RECURSE MOCK_SRCS "mocks/*.cpp")

add_executable(runner 
    ${TEST_SRCS} 
    ${PROJ_SRCS}
)

target_compile_features(runner PRIVATE cxx_std_23)

target_include_directories(runner PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${doctest_SOURCE_DIR}/doctest 
)



target_link_libraries(runner PRIVATE 
    doctest::doctest
    bme280
)

enable_testing()
include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(runner)

# Convenience target to build and run tests
add_custom_target(test 
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --verbose
    DEPENDS runner
    USES_TERMINAL
)

