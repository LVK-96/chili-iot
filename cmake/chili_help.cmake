# Helper macro to add help line if target exists
set(HELP_COMMANDS 
    COMMAND ${CMAKE_COMMAND} -E echo "================================================================="
    COMMAND ${CMAKE_COMMAND} -E echo "                        CHILI IOT BUILD SYSTEM"
    COMMAND ${CMAKE_COMMAND} -E echo "================================================================="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "AVAILABLE TARGETS:"
)

function(add_help_line target_name description)
    # Check if target exists OR if it's a known standard target name we want to enforce
    if(TARGET ${target_name} OR "${target_name}" MATCHES "^(clean|test|install)$")
        list(APPEND HELP_COMMANDS COMMAND ${CMAKE_COMMAND} -E echo "${target_name} \t- ${description}")
        set(HELP_COMMANDS ${HELP_COMMANDS} PARENT_SCOPE)
    endif()
endfunction()

# Register known targets
add_help_line("chili-help"       "Show this help message")
add_help_line("sensor_node"      "Build Firmware (ELF + BIN)")
add_help_line("test"             "Run all Unit Tests")
add_help_line("runner"           "Build Test Executable")
add_help_line("style-check"      "Check code formatting")
add_help_line("style-fix"        "Auto-fix code formatting")
add_help_line("clang-tidy"       "Run Static Analysis")
add_help_line("clean"            "Remove build artifacts")
add_help_line("disassemble"      "Generate assembly listing")
add_help_line("gdb-server"       "Start GDB Server")
add_help_line("mqtt-broker"      "Run MQTT Test Broker")
add_help_line("udp-server"       "Run UDP Test Server")

list(APPEND HELP_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "FLASHING:"
    COMMAND ${CMAKE_COMMAND} -E echo "  ./flash.sh            - Flash firmware to device"
    COMMAND ${CMAKE_COMMAND} -E echo "================================================================="
)

add_custom_target(chili-help
    ${HELP_COMMANDS}
    COMMENT "Listing available Chili IoT targets..."
    VERBATIM
)
