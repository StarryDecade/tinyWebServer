#pragma once

#include <sys/time.h>

#include "locker.h"

template <typename T>
class block_queue {
 public:
    block_queue(int max_size = 1000);
    ~block_queue();

    void clear();
    int size();
    int max_size();
    bool full();
    bool empty();
    bool front(T &value);
    bool back(T &value);
    bool push(const T &item);
    bool pop(T &item);
    bool pop(T &item, int ms_timeout);

 private:
    locker m_mutex;
    cond m_cond;

    T *m_arry;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};
