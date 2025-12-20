#!/bin/bash
set -e

# Default paths
DEFAULT_BINARY="build/arm/firmware.bin"
DEBUG_BINARY="build/arm/firmware-debug.bin"

BINARY="$DEFAULT_BINARY"
ADDRESS="0x8000000"

# Argument parsing
if [ "$1" == "--semihosting" ]; then
    BINARY="$DEBUG_BINARY"
elif [ -n "$1" ]; then
    BINARY="$1"
fi

if ! command -v st-flash &> /dev/null; then
    echo "Error: st-flash not found!"
    echo "Please install stlink tools (e.g. 'sudo apt install stlink-tools' on Ubuntu/Debian)."
    exit 1
fi

if [ ! -f "$BINARY" ]; then
    echo "Error: Firmware binary not found at $BINARY"
    echo "Usage: $0 [--semihosting | <custom_binary_path>]"
    exit 1
fi

echo "Flashing $BINARY to $ADDRESS..."
st-flash --connect-under-reset write "$BINARY" "$ADDRESS"
echo "Done."
