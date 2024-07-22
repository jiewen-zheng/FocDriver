/**
 * @file MLog.cpp
 */

#include "MLog.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>



/**
 * Add a log
 * @param level the level of log. (From `lv_log_level_t` enum)
 * @param file name of the file when the log added
 * @param line line number in the source code where the log added
 * @param func name of the function when the log added
 * @param format printf-like format string
 * @param ... parameters for `format`
 */
void MLog::add(MLogLevel_t level, const char *file, int line, const char *func, const char *format, ...) {
    static const char *log_prefix[] = {"Trace", "Info", "Warn", "Error", "User", "None"};

    if (level >= _LOG_LEVEL_LAST) return;  /*Invalid level*/

    if (level < _level) return;

    static uint32_t last_log_time = 0;

    va_list args;
    va_start(args, format);


    /*Use only the file name not the path*/
    size_t p;
    for(p = strlen(file); p > 0; p--) {
        if(file[p] == '/' || file[p] == '\\') {
            p++;    /*Skip the slash*/
            break;
        }
    }


    uint32_t time = _tick != nullptr ? _tick() : 0;

    if (callback) {
        char msg[MSG_SIZE];
        snprintf(msg, MSG_SIZE, "[%s]\t(time:%d.%03d, +%d)\t(file: %s line #%d)\t %s: ",
                 log_prefix[level], time / 1000, time % 100, time - last_log_time, &file[p], line, func);

        uint32_t len = strlen(msg);
        vsnprintf(msg + len, MSG_SIZE - len, format, args);
        strcat(msg, "\n");
        callback(msg);
    } else {
        printf("[%s]\t(time:%d.%03d, +%d)\t(file: %s line #%d)\t %s: ",
               log_prefix[level], time / 1000, time % 1000, time - last_log_time, &file[p], line, func);
        vprintf(format, args);
        printf("\n");
    }

    last_log_time = time;
    va_end(args);
}

void MLog::addNoPrefix(const char *format, ...) {
    va_list args;
    va_start(args, format);

    vprintf(format, args);
    printf("\n");

    va_end(args);

}

#if M_LOG_USED
MLog m_log;
#endif
