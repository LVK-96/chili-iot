#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#include <cstdlib>

extern "C" void initialise_monitor_handles();

int main(int argc, char** argv)
{
    initialise_monitor_handles();

    doctest::Context context;
    context.applyCommandLine(argc, argv);

    int res = context.run();

    exit(res); // Trigger semihosting exit
}
