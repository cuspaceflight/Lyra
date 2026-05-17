#include "logger.h"

#include <pico/time.h>
#include <stdarg.h>

#include <stdio.h>

#ifdef LOG_ENABLE

void logger_print(enum log_level level, const char* tag, const char* func, const char* message, ...)
{
    absolute_time_t time = get_absolute_time();
    uint32_t        ms   = to_ms_since_boot(time);

    static const char* STR_LEVELS[] = {
        "",
        " VERBOSE",
        "   DEBUG",
        "    INFO",
        "   ERROR",
        "CRITICAL",
    };

#ifdef LOG_ENABLE_COLOUR
    static const char* STR_COLOURS[] = {
        "",
        "\e[0;37m",
        "\e[0;36m",
        "\e[0;33m",
        "\e[0;32m",
        "\e[0;35m",
        "\e[0;31m",
    };
#endif

    if (level < LOG_LEVEL) {
        return;
    }

#ifdef LOG_ENABLE_COLOUR
    printf("%s", STR_COLOURS[level]);
#endif

    printf(LOG_FORMAT, STR_LEVELS[level], ms, tag, func);
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);

#ifdef LOG_ENABLE_COLOUR
    printf("\e[0m");
#endif

    printf("\n");
}

#endif
