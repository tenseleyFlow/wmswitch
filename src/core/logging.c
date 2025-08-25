#include "wmswitch/logging.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

static wmswitch_log_level_t current_log_level = WMSWITCH_LOG_INFO;

static const char* log_level_names[] = {
    "ERROR",
    "WARN ",
    "INFO ",
    "DEBUG"
};

static const char* log_level_colors[] = {
    "\033[31m", // Red for ERROR
    "\033[33m", // Yellow for WARNING  
    "\033[32m", // Green for INFO
    "\033[36m"  // Cyan for DEBUG
};

static const char* color_reset = "\033[0m";

void wmswitch_log_init(wmswitch_log_level_t level) {
    current_log_level = level;
}

void wmswitch_log_set_level(wmswitch_log_level_t level) {
    current_log_level = level;
}

void wmswitch_log(wmswitch_log_level_t level, const char *file, int line, const char *fmt, ...) {
    if (level > current_log_level) {
        return;
    }
    
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    
    const char *filename = strrchr(file, '/');
    filename = filename ? filename + 1 : file;
    
    fprintf(stderr, "%s[%02d:%02d:%02d] [%s] %s:%d: ",
            log_level_colors[level],
            local->tm_hour, local->tm_min, local->tm_sec,
            log_level_names[level],
            filename, line);
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    
    fprintf(stderr, "%s\n", color_reset);
    fflush(stderr);
}