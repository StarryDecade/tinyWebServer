#pragma once

#include "locker.h"

template <typename T>
class block_queue {
 public:
    block_queue();
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
    locker m_lock;
    cond m_cond;

    T *m_arry;
    int m_size;
    int m_max_size;
    int m_front;
    int m_back;
};
