#ifndef MYSERVER_INCLUDE_LOG_H
#define MYSERVER_INCLUDE_LOG_H

#ifdef DEBUG
#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#else
#define LOG_DEBUG(format, ...)
#define LOG_INFO(format, ...)
#define LOG_WARN(format, ...)
#define LOG_ERROR(format, ...)
#endif

namespace myserver::utils {

}

#endif // MYSERVER_INCLUDE_LOG_H