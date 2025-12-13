#include "System.h"
#include "USART.h"

extern "C" {

int _write([[maybe_unused]] int file, char* ptr, int len)
{
    bluepill::peripherals::usart1.send_blocking(std::as_bytes(std::span(ptr, static_cast<size_t>(len))));
    return len;
}

int _close([[maybe_unused]] int file) { return -1; }
int _fstat([[maybe_unused]] int file, [[maybe_unused]] void* st) { return 0; }
int _isatty([[maybe_unused]] int file) { return 1; }
int _lseek([[maybe_unused]] int file, [[maybe_unused]] int ptr, [[maybe_unused]] int dir) { return 0; }
int _read([[maybe_unused]] int file, [[maybe_unused]] char* ptr, [[maybe_unused]] int len) { return 0; }
int _getpid(void) { return 1; }
int _kill([[maybe_unused]] int pid, [[maybe_unused]] int sig) { return -1; }
}
