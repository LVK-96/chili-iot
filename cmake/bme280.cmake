include_guard(GLOBAL)
# BME280 wrapper

# BME280 driver is just source files
add_library(bme280 STATIC
    ${bme280_driver_SOURCE_DIR}/bme280.c
)

target_include_directories(bme280 PUBLIC ${bme280_driver_SOURCE_DIR})

# Suppress known warnings in vendor code if needed
target_compile_options(bme280 PRIVATE -Wno-missing-field-initializers)
