#!/bin/bash
set -e

# BINARY_PATH matches the path in CMakeLists.txt and Docker build
BINARY="build/stm32/sensor_node.bin"
ADDRESS="0x8000000"

if ! command -v st-flash &> /dev/null; then
    echo "Error: st-flash not found!"
    echo "Please install stlink tools (e.g. 'sudo apt install stlink-tools' on Ubuntu/Debian)."
    exit 1
fi

if [ ! -f "$BINARY" ]; then
    echo "Error: Firmware binary not found at $BINARY"
    echo "Please build the project inside Docker first: ./docker-build.sh cmake --build build/stm32"
    exit 1
fi

echo "Flashing $BINARY to $ADDRESS..."
st-flash --connect-under-reset write "$BINARY" "$ADDRESS"
echo "Done."
