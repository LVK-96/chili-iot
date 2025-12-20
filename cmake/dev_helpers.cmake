include_guard(GLOBAL)
# Development Helper Targets


find_program(PYTHON3 python3)
if(PYTHON3)
    # --- UDP Test Server ---
    # Runs scripts/esp8266_test/udp_server.py
    # Uses SECRET_SERVER_IP and SECRET_SERVER_PORT defined in cmake/setup_secrets.cmake
    add_custom_target(udp-server
        COMMAND ${CMAKE_COMMAND} -E env
            UDP_SERVER_IP=${SECRET_SERVER_IP}
            UDP_SERVER_PORT=${SECRET_SERVER_PORT}
            ${PYTHON3} ${CMAKE_SOURCE_DIR}/scripts/esp8266_test/udp_server.py
        USES_TERMINAL
    )
    # --- MQTT Broker ---
    # Runs scripts/esp8266_test/run_broker.py
    # Uses SECRET_SERVER_IP and SECRET_SERVER_PORT defined in cmake/setup_secrets.cmake
    add_custom_target(mqtt-broker
        COMMAND ${CMAKE_COMMAND} -E env
            BROKER_IP=${SECRET_SERVER_IP}
            BROKER_PORT=${SECRET_SERVER_PORT}
            ${PYTHON3} ${CMAKE_SOURCE_DIR}/scripts/esp8266_test/run_broker.py
        USES_TERMINAL
    )
    # --- MQTT Subscriber ---
    # Runs scripts/esp8266_test/mqtt_subscribe.py
    # Uses SECRET_SERVER_IP and SECRET_SERVER_PORT defined in cmake/setup_secrets.cmake
    add_custom_target(mqtt-subscribe
        COMMAND ${CMAKE_COMMAND} -E env
            BROKER_IP=${SECRET_SERVER_IP}
            BROKER_PORT=${SECRET_SERVER_PORT}
            ${PYTHON3} ${CMAKE_SOURCE_DIR}/scripts/esp8266_test/mqtt_subscribe.py
        USES_TERMINAL
    )
endif()
