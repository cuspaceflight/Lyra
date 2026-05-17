#pragma once

enum log_level {
    LOG_LEVEL_VERBOSE  = 1,
    LOG_LEVEL_DEBUG    = 2,
    LOG_LEVEL_INFO     = 3,
    LOG_LEVEL_ERROR    = 4,
    LOG_LEVEL_CRITICAL = 5,
    LOG_LEVEL_OFF      = 6,
};

#ifdef LOG_ENABLE

#ifndef LOG_FORMAT
#define LOG_FORMAT "[%s][%7lu][%s][%s]: "
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_ERROR
#endif

void logger_print(
    enum log_level level, const char* tag, const char* func, const char* message, ...);

#define LOG_VERBOSE(TAG, ...)  logger_print(LOG_LEVEL_VERBOSE, TAG, __func__, __VA_ARGS__)
#define LOG_DEBUG(TAG, ...)    logger_print(LOG_LEVEL_DEBUG, TAG, __func__, __VA_ARGS__)
#define LOG_INFO(TAG, ...)     logger_print(LOG_LEVEL_INFO, TAG, __func__, __VA_ARGS__)
#define LOG_ERROR(TAG, ...)    logger_print(LOG_LEVEL_ERROR, TAG, __func__, __VA_ARGS__)
#define LOG_CRITICAL(TAG, ...) logger_print(LOG_LEVEL_CRITICAL, TAG, __func__, __VA_ARGS__)

#else

#define LOG_VERBOSE(...)
#define LOG_DEBUG(...)
#define LOG_INFO(...)
#define LOG_ERROR(...)
#define LOG_CRITICAL(...)

#endif
