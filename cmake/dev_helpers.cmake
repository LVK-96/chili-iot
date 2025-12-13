# Development Helper Targets

# --- UDP Test Server ---
# Runs scripts/esp8266_test/udp_server.py
# Uses SECRET_SERVER_IP and SECRET_SERVER_PORT defined in cmake/setup_secrets.cmake

find_program(PYTHON3 python3)
if(PYTHON3)
    add_custom_target(udp-server
        COMMAND ${CMAKE_COMMAND} -E env
            UDP_SERVER_IP=${SECRET_SERVER_IP}
            UDP_SERVER_PORT=${SECRET_SERVER_PORT}
            ${PYTHON3} ${CMAKE_SOURCE_DIR}/scripts/esp8266_test/udp_server.py
        USES_TERMINAL
    )
endif()
