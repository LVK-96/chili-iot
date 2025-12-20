include_guard(GLOBAL)
include(FetchContent)

# --- libopencm3 ---
FetchContent_Declare(
    libopencm3
    GIT_REPOSITORY https://github.com/libopencm3/libopencm3.git
    GIT_TAG        1f3abd43763fa39d23e737602b6d0011a45c70b2
)

# --- FreeRTOS Kernel ---
FetchContent_Declare(
    FreeRTOS-Kernel
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
    GIT_TAG        18ed8886fe3eed9dbe433a82cdc62ce5dc0315eb
)

# --- BME280 Driver ---
FetchContent_Declare(
    BME280_driver
    GIT_REPOSITORY https://github.com/BoschSensortec/BME280_driver.git
    GIT_TAG        07a4960f31956ceee8ed071c9074c1cf05b68e76
)

# --- doctest ---
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG        v2.4.11
)
