//
// Created by admin on 2024/5/15.
//

#ifndef _M_LOG_H
#define _M_LOG_H

#include <stdint.h>

#define M_LOG_USED      1


typedef enum : uint8_t {
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_USER,
    LOG_LEVEL_NONE,
    _LOG_LEVEL_LAST,
} MLogLevel_t;


class MLog {
#define MSG_SIZE    256

    typedef void    (*LogCallback_t)(const char *buf);
    typedef uint32_t(*GetTick_t)();

public:
    explicit MLog(MLogLevel_t level = LOG_LEVEL_TRACE, LogCallback_t cb = nullptr, GetTick_t tick = nullptr) :
            _level(level), callback(cb), _tick(tick) {};

    ~MLog() = default;

    void setLevel(MLogLevel_t level) { _level = level; }
    void setPrintCb(LogCallback_t cb) { callback = cb; }
    void setTickCallback(GetTick_t tick) { _tick = tick; }

    void add(MLogLevel_t level, const char *file, int line, const char *func, const char *format, ...);
    static void addNoPrefix(const char *format, ...);

private:
    MLogLevel_t   _level;
    GetTick_t     _tick;
    LogCallback_t callback;
};


/**********************
 *      MACROS
 **********************/
#if M_LOG_USED

extern MLog m_log;


#ifndef M_LOG_TRACE
#   define M_LOG_TRACE(...) m_log.add(LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#ifndef M_LOG_INFO
#   define M_LOG_INFO(...)  m_log.add(LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#ifndef M_LOG_WARN
#   define M_LOG_WARN(...)  m_log.add(LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#ifndef M_LOG_ERROR
#   define M_LOG_ERROR(...) m_log.add(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#ifndef M_LOG_USER
#   define M_LOG_USER(...)  m_log.add(LOG_LEVEL_USER, __FILE__, __LINE__, __func__, __VA_ARGS__)
#endif

#ifndef M_LOG
#   define M_LOG(...)       m_log.addNoPrefix(__VA_ARGS__)
#endif

#else   // M_LOG_USED

/*Do nothing if `LV_USE_LOG 0`*/
#define M_LOG_TRACE(...)    do {}while(0)
#define M_LOG_INFO(...)     do {}while(0)
#define M_LOG_WARN(...)     do {}while(0)
#define M_LOG_ERROR(...)    do {}while(0)
#define M_LOG_USER(...)     do {}while(0)
#define M_LOG(...)          do {}while(0)

#endif  // M_LOG_USED

#endif // _M_LOG_H
