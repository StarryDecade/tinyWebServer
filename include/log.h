#pragma once

#include <stdarg.h>
#include <stdio.h>

#include <string>

#include "block_queue.h"

using namespace std;

class Log {
public:
    // 懒汉模式
    static Log *get_instance() {
        static Log instance;
        return &instance;
    }

    // 异步写日志的共有方法， 调用私有方法
    static void *flush_log_thread(void *args) {
        Log::get_instance()->async_write_log();
        return NULL;
    }

    bool init(const char *file_name, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush();

private:
    Log();
    ~Log();
    void *async_write_log() {
        string single_log;
        while (m_log_queue->pop(single_log)) {
            m_mutex.lock();
            fputs(single_log.c_str(), m_fp);
            m_mutex.unlock();
        }
        return NULL;
    }

private:
    char dir_name[128];
    char log_name[128];
    int m_split_lines;
    int m_log_buf_size;
    long long m_count;  // 日志行数记录
    int m_today;
    FILE *m_fp;
    char *m_buf;
    block_queue<string> *m_log_queue;
    bool m_is_async;  // 异步标志
    locker m_mutex;
    int m_close_log;
};

#define LOG_DEBUG(format, ...) Log::get_instance()->write_log(0, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) Log::get_instance()->write_log(1, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) Log::get_instance()->write_log(2, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) Log::get_instance()->write_log(3, format, ##__VA_ARGS__)
