# Chili IoT - STM32 Sensor Node

An embedded IoT sensor node project for STM32F103C8T6 (Blue Pill) featuring FreeRTOS, libopencm3, MQTT communication, and BME280 environmental sensing.

## Hardware

- **MCU**: STM32F103C8T6 (Cortex-M3)
- **Sensor**: BME280 (Temperature, Humidity, Pressure)
- **Programmer**: ST-Link V2

## Containerized Build System

This project provides a containerized build environment to ensure consistent environment across all machines.

### 1. Prerequisites

- Podman.
- `stlink-tools` on host (for flashing)

### 2. Quick Start (Helper Script)

The `container-build.sh` script automates building and running the container.

```bash
# Build Firmware (inside container)
./container-build.sh cmake --preset target && ./container-build.sh cmake --build build/stm32

# Build Tests (inside container)
./container-build.sh cmake --preset host && ./container-build.sh cmake --build build/host
```

### 3. Flashing

Flashing is done from the **host machine** to preserve portability.

```bash
# Flash the binary built by Podman
./flash.sh
```

## Environment setup (Manual)

### System Packages (Debian/Ubuntu)

```bash
# Toolchain and build tools
sudo apt-get install \
    gcc-arm-none-eabi \
    cmake \
    ninja-build \
    stlink-tools \
    python3 \
    python3-venv \
    python3-pip

# Optional (for code quality)
sudo apt-get install clang-format clang-tidy
```

### Python environment

```bash
# Create and activate virtual environment
python3 -m venv .venv
source .venv/bin/activate

# Install dependencies
pip install -r requirements.txt
```

## Build System

This project uses **CMake Presets** for configuration:

### Available Presets

| Preset | Description | Binary Dir |
|--------|-------------|------------|
| `host` | Native tests on your machine | `build/host` |
| `target` (or `stm32`) | Cross-compile for STM32 | `build/stm32` |

## Project Structure

```text
chili-iot/
├── src/              # Firmware source code
├── tests/            # Unit tests
├── cmake/            # CMake modules and toolchain files
├── scripts/          # Utility scripts
├── deps/             # Auto-fetched dependencies (cached)
├── build/
│   ├── host/        # Test build artifacts
│   └── stm32/       # Firmware build artifacts
└── CMakePresets.json # Build configurations
```

## Development Workflow

### Running Tests

```bash
./container-build.sh cmake --preset host
./container-build.sh cmake --build build/host --target test
```

### Building Firmware

```bash
./container-build.sh cmake --preset target
./container-build.sh cmake --build build/stm32
```

### Flashing

```bash
# Flash the firmware (using helper script)
./flash.sh
```

### Code Quality

```bash
# Check formatting
./container-build.sh cmake --build build/host --target style-check

# Auto-fix formatting
./container-build.sh cmake --build build/host --target style-fix

# Run static analysis
./container-build.sh cmake --build build/host --target clang-tidy
```

### Clean Builds

```bash
# Clean specific build
./container-build.sh cmake --build build/host --target clean
./container-build.sh cmake --build build/stm32 --target clean

# Or delete entire build directory (on host)
rm -rf build
```

### Checking Available Targets

To see a list of all build targets for your current configuration (e.g., `host`), run:

```bash
# List available targets
./container-build.sh cmake --build build/host --target chili-help
```

## Configuration

### Secrets

WiFi credentials and other secrets are managed via a `.secrets` file in the project root (not tracked in git). Create this file with:

```ini
WIFI_SSID = "your-ssid"
WIFI_PASSWORD = "your-password"
SERVER_IP = "your-ip"
SERVER_PORT = "your-port"
```

These are processed by `cmake/setup_secrets.cmake` and injected at compile time using `-include` to keep them out of build logs.

## Troubleshooting

### Permission denied when flashing

Add your user to the dialout group:

```bash
sudo usermod -a -G dialout $USER
# Log out and back in
```

## License

TBD
