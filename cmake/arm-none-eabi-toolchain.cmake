include_guard(GLOBAL)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Toolchain settings
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
# Find toolchain binaries
find_program(CMAKE_OBJCOPY arm-none-eabi-objcopy)
find_program(CMAKE_SIZE arm-none-eabi-size)
find_program(CMAKE_OBJDUMP arm-none-eabi-objdump)
find_program(CMAKE_AR arm-none-eabi-ar)

# Warn if tools are missing
if(NOT CMAKE_OBJCOPY)
    message(WARNING "arm-none-eabi-objcopy not found! Binary generation will fail.")
endif()
if(NOT CMAKE_SIZE)
    message(WARNING "arm-none-eabi-size not found! Size reporting will fail.")
endif()

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Compiler flags
set(CPU_FLAGS "-mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd -msoft-float")

set(CMAKE_C_FLAGS "${CPU_FLAGS} -ffunction-sections -fdata-sections -fno-exceptions" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${CPU_FLAGS} -ffunction-sections -fdata-sections -fno-use-cxa-atexit -fno-rtti -fno-exceptions" CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${CPU_FLAGS}" CACHE INTERNAL "ASM Compiler options")
set(CMAKE_EXE_LINKER_FLAGS "${CPU_FLAGS} -Wl,--gc-sections -static -nostartfiles --specs=nano.specs --specs=nosys.specs" CACHE INTERNAL "Linker options")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
