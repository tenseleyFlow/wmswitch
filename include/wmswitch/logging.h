#ifndef WMSWITCH_LOGGING_H
#define WMSWITCH_LOGGING_H

typedef enum {
    WMSWITCH_LOG_ERROR = 0,
    WMSWITCH_LOG_WARNING = 1,
    WMSWITCH_LOG_INFO = 2,
    WMSWITCH_LOG_DEBUG = 3
} wmswitch_log_level_t;

void wmswitch_log_init(wmswitch_log_level_t level);
void wmswitch_log_set_level(wmswitch_log_level_t level);
void wmswitch_log(wmswitch_log_level_t level, const char *file, int line, const char *fmt, ...);

#define wmswitch_log_error(fmt, ...) \
    wmswitch_log(WMSWITCH_LOG_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define wmswitch_log_warning(fmt, ...) \
    wmswitch_log(WMSWITCH_LOG_WARNING, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define wmswitch_log_info(fmt, ...) \
    wmswitch_log(WMSWITCH_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#define wmswitch_log_debug(fmt, ...) \
    wmswitch_log(WMSWITCH_LOG_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif