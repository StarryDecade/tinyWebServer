#include <pthread.h>

#include <exception>

#include "locker.h"
#include "sql_connection_pool.h"

template <typename T>
class threadpool {
 public:
    threadpool(connection_pool *connPool, int thread_number = 8, int max_requests = 1000);
    ~threadpool();
    bool appent(T *request);

 private:
    static void *worker(void *arg);
    void run();

 private:
    int m_thread_number;
    int m_max_requests;
    pthread_t *m_threads;
    list<T *> m_workerQueue;
    locker m_queueLocker;
    sem m_queueStat;
    bool m_stop;
    connection_pool *m_connPool;
};

template <typename T>
threadpool<T>::threadpool(connection_pool *connPool, int thread_number, int max_requests) : m_thread_number(thread_number), m_max_requests(max_requests), m_threads(NULL), m_stop(false), m_connPool(connPool) {
    if (m_thread_number <= 0 || m_max_requests <= 0) throw exception();

    m_threads = new pthread_t(m_thread_number);
    if (!m_threads) throw exception();
    for (int i = 0; i < m_max_requests; i++) {
        if (pthread_create(m_threads + i, NULL, worker, this)) {
            delete[] m_threads;
            throw exception();
        }
        if (pthread_detach(m_threads[i])) {
            delete[] m_threads;
            throw exception();
        }
    }
}

template <typename T>
threadpool<T>::~threadpool() {
    delete[] m_threads;
    m_stop = true;
}

template <typename T>
bool threadpool<T>::appent(T *request) {
    m_queueLocker.lock();
    if (m_workerQueue.size() > m_max_requests) {
        m_queueLocker.unlock();
        return false;
    }
    m_workerQueue.push_back(request);
    m_queueLocker.unlock();
    m_queueStat.post();
    return true;
}

template <typename T>
void *threadpool<T>::worker(void *arg) {
    threadpool *pool = (threadpool *)arg;
    pool->run();
    return pool;
}

template <typename T>
void threadpool<T>::run() {
    while (!m_stop) {
        m_queueStat.wait();
        m_queueLocker.lock();
        if (m_workerQueue.empty()) {
            m_queueLocker.unlock();
            continue;
        }
        T *request = m_workerQueue.front();
        m_workerQueue.pop_front();
        m_queueLocker.unlock();
        if (!request) continue;

        connectionRAII mysqlConn(&request->mysql, m_connPool);

        request->process();
    }
}
