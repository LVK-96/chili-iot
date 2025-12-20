# Chili IoT - STM32 Sensor Node

An embedded IoT sensor node project for STM32F103C8T6 (Blue Pill) featuring FreeRTOS, libopencm3, MQTT communication, and BME280 environmental sensing.

## Hardware

- **MCU**: STM32F103C8T6 (Cortex-M3)
- **Sensor**: BME280 (Temperature, Humidity, Pressure)
- **Programmer**: ST-Link V2

## Containerized Build System

This project provides a containerized build to ensure consistent environment across all machines.

### Prerequisites

- Podman
- `stlink-tools` on host (for flashing)

### Quick Start

```bash
# Configure and build firmware
./container-build.sh cmake --preset arm
./container-build.sh cmake --build build/arm --target firmware

# Configure and build on host unit tests
./container-build.sh cmake --preset host
./container-build.sh cmake --build build/host --target test
```

## Build System

This project uses **CMake Presets** organized by CPU architecture:

### Presets

| Preset | Architecture | Description | Binary Dir |
|--------|--------------|-------------|------------|
| `arm` | ARM Cortex-M | Cross-compile for STM32. Builds firmware and tests. | `build/arm` |
| `host` | Host machine | Native build for host machine. Unit tests only. | `build/host` |

### ARM Targets (`--preset arm`)

| Target | Description |
|--------|-------------|
| `firmware` | Production build |
| `firmware-debug` | Semihosting enabled firmware |
| `qemu-test` | Run unit tests in QEMU  |
| `qemu-sim` | Run semihosting firmware in QEMU |
| `flash` | Flash firmware to device |
| `flash-debug` | Flash semihosting firmware to device |

### Host Targets (`--preset host`)

| Target | Description |
|--------|-------------|
| `runner` | Native unit tests executable |
| `test` | Run unit tests |

## Project Structure

```text
chili-iot/
├── src/              # Firmware source code
├── tests/            # Unit tests and mocks
├── cmake/            # CMake modules and toolchain files
├── deps/             # Auto-fetched dependencies (cached)
├── build/
│   ├── arm/         # ARM build artifacts (firmware + tests)
│   └── host/        # Host build artifacts (tests only)
└── CMakePresets.json # Build configurations
```

## Development Workflow

### Building Firmware

```bash
# Configure (once)
./container-build.sh cmake --preset arm

# Build production firmware
./container-build.sh cmake --build build/arm --target firmware

# Build debug firmware (semihosting for debugger)
./container-build.sh cmake --build build/arm --target firmware-debug
```

### Running in QEMU

The mps2-an385 device is used to model a generic Cortex-M3 device. All the interactions with bluepill peripherals are mocked in QEMU simulation.

```bash
# Run firmware simulation
./container-build.sh cmake --build build/arm --target qemu-firmware

# Run unit tests in QEMU
./container-build.sh cmake --build build/arm --target qemu-test
```

To exit QEMU: press `Ctrl+X X` (sends escape character).

### Running Host Tests

```bash
# Configure (once)
./container-build.sh cmake --preset host

# Build and run tests
./container-build.sh cmake --build build/host --target test
```

### Flashing

Outside the build container.

```bash
./flash.sh
```

### Code Quality

```bash
# Check formatting
./container-build.sh cmake --build build/arm --target style-check

# Auto-fix formatting
./container-build.sh cmake --build build/arm --target style-fix

# Run static analysis
./container-build.sh cmake --build build/arm --target clang-tidy
```

### Clean Builds

```bash
# Clean specific build
./container-build.sh cmake --build build/arm --target clean

# Or just nuke the entire build directory
rm -rf build
```

### Available Targets

```bash
# List all available targets
./container-build.sh cmake --build build/arm --target help
# OR
./container-build.sh cmake --build build/host --target help
```

## Environment Setup (Manual)

Note, the build might depend on some newish features of the tools o your milage may vary depending on your system.

### System Packages (Debian/Ubuntu)

```bash
# Toolchain and build tools
sudo apt-get install \
    gcc-arm-none-eabi \
    cmake \
    ninja-build \
    stlink-tools \
    qemu-system-arm \
    python3 \
    python3-venv \
    python3-pip

# Optional (for code quality)
sudo apt-get install clang-format clang-tidy
```

### Python Environment

```bash
# Create and activate virtual environment
python3 -m venv .venv
source .venv/bin/activate

# Install dependencies
pip install -r requirements.txt
```

## Configuration

### Secrets

WiFi credentials and other secrets are managed via a `.secrets` file.

```ini
WIFI_SSID = "your-ssid"
WIFI_PASSWORD = "your-password"
SERVER_IP = "your-ip"
SERVER_PORT = "your-port"
```

These are processed by `cmake/setup_secrets.cmake` and injected at compile time.

## Troubleshooting

### Permission denied when flashing

Add your user to the dialout group:

```bash
sudo usermod -a -G dialout $USER
# Log out and back in
```

## License

TBD
