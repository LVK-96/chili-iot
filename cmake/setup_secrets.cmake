# Track .secrets file so CMake re-runs if it changes
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/.secrets")

set(SECRETS_FILE "${CMAKE_SOURCE_DIR}/.secrets")
set(GENERATED_SECRETS_H "${CMAKE_BINARY_DIR}/generated_secrets.h")

# Initialize files
file(WRITE "${GENERATED_SECRETS_H}" "// Auto-generated secrets from .secrets\n// Included via -include flag to hide from command line\n")

# Default vars for script
set(SECRET_SERVER_IP "")
set(SECRET_SERVER_PORT "")

if(EXISTS ${SECRETS_FILE})
    file(STRINGS ${SECRETS_FILE} SECRET_LINES)
    foreach(LINE ${SECRET_LINES})
        # Match lines like VAR = "value", VAR = value, or VAR := value
        if(LINE MATCHES "^[ \t]*([a-zA-Z0-9_]+)[ \t]*[:]?=[ \t]*(.*)$")
            set(KEY "${CMAKE_MATCH_1}")
            set(VALUE "${CMAKE_MATCH_2}")
            
            # Remove backslash-escapes from quotes if present (e.g. from file(STRINGS) or bad input)
            string(REPLACE "\\\"" "\"" VALUE "${VALUE}")

            # Write to header (Preserve quotes if present, as C++ strings need them)
            file(APPEND "${GENERATED_SECRETS_H}" "#define ${KEY} ${VALUE}\n")
            
            # Clean quotes for usage in script/logic if needed
            string(REPLACE "\"" "" VALUE_CLEAN "${VALUE}")
            
            if(KEY STREQUAL "SERVER_IP")
                set(SECRET_SERVER_IP "${VALUE_CLEAN}")
            elseif(KEY STREQUAL "SERVER_PORT")
                set(SECRET_SERVER_PORT "${VALUE_CLEAN}")
            endif()
        endif()
    endforeach()
endif()

# Appending script command


# Export variable for usage in CMakeLists.txt
# We use -include to force inclusion, avoiding -D flags in process tree
set(SECRETS_COMPILE_OPTIONS "-include" "${GENERATED_SECRETS_H}")
