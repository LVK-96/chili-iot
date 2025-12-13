# Chili IoT - STM32 Sensor Node

An embedded IoT sensor node project for STM32F103C8T6 (Blue Pill) featuring FreeRTOS, MQTT communication, and BME280 environmental sensing.

## Hardware

- **MCU**: STM32F103C8T6 (Cortex-M3)
- **Sensor**: BME280 (Temperature, Humidity, Pressure)
- **Programmer**: ST-Link V2

## Dependencies

### System Packages

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

### Firmware Dependencies (Auto-fetched)

The following are automatically downloaded via CMake `FetchContent`:

- **libopencm3** - STM32 peripheral library
- **FreeRTOS-Kernel** - Real-time operating system
- **BME280_driver** - Bosch sensor driver

### Test Dependencies (Auto-fetched)

- **doctest** - C++ testing framework

### Python Dependencies

```bash
pip install -r requirements.txt
```

## Quick Start

### 1. Environment Setup

```bash
source setup_env
```

This will:

- Create and activate a Python virtual environment
- Install Python dependencies
- Set Ninja as the default CMake generator (if available)

### 2. Build and Run Tests

```bash
cmake --preset host
cmake --build build/host --target test
```

### 3. Build Firmware

```bash
cmake --preset target
cmake --build build/stm32
```

### 4. Flash to Device

Connect your ST-Link and Blue Pill, then:

```bash
cmake --build build/stm32 --target flash
```

## Build System

This project uses **CMake Presets** for configuration:

### Available Presets

| Preset | Description | Binary Dir |
|--------|-------------|------------|
| `host` | Native tests on your machine | `build/host` |
| `target` (or `stm32`) | Cross-compile for STM32 | `build/stm32` |

### Available Targets

#### Universal (Both Presets)

| Target | Description |
|--------|-------------|
| `udp-server` | Run UDP test server script |
| `style-check` | Check code formatting (requires clang-format) |
| `style-fix` | Auto-fix code formatting |
| `clang-tidy` | Static analysis (requires clang-tidy) |
| `clang-tidy-fix` | Auto-fix static analysis issues |
| `clean` | Remove build artifacts |
| `help` | List all targets |

#### Host Only

| Target | Description |
|--------|-------------|
| `test` | Build and run all unit tests |
| `runner` | Test executable binary |

#### Firmware Only

| Target | Description |
|--------|-------------|
| `sensor_node` | Main firmware binary (ELF + BIN) |
| `flash` | Flash firmware to device (requires st-flash) |
| `disassemble` | Generate assembly dump |
| `gdb-server` | Start GDB server for debugging |

## Project Structure

```
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
cmake --preset host
cmake --build build/host --target test
```

### Building Firmware

```bash
cmake --preset target
cmake --build build/stm32
```

### Flashing

```bash
# Flash the firmware
cmake --build build/stm32 --target flash

# Start GDB server for debugging
cmake --build build/stm32 --target gdb-server
```

### Code Quality

```bash
# Check formatting
cmake --build build/host --target style-check

# Auto-fix formatting
cmake --build build/host --target style-fix

# Run static analysis (if clang-tidy installed)
cmake --build build/host --target clang-tidy
```

### Clean Builds

```bash
# Clean specific build
cmake --build build/host --target clean
cmake --build build/stm32 --target clean

# Or delete entire build directory
rm -rf build
```

## Configuration

### Secrets

WiFi credentials and other secrets are managed via a `.secrets` file in the project root (not tracked in git). Create this file with:

```
WIFI_SSID = "your-ssid"
WIFI_PASSWORD = "your-password"
SERVER_IP = "your-ip"
SERVER_PORT = "your-port"
```

These are processed by `cmake/setup_secrets.cmake` and injected at compile time using `-include` to keep them out of build logs.

## Troubleshooting

### st-flash not found

```bash
sudo apt-get install stlink-tools
```

### Ninja not found

```bash
sudo apt-get install ninja-build
```

Or use Make instead (CMake will auto-detect):

```bash
cmake --preset target
```

### Permission denied when flashing

Add your user to the dialout group:

```bash
sudo usermod -a -G dialout $USER
# Log out and back in
```

## License

TBD
