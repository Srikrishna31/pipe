#pragma once

#include <chrono>
#include <thread>
#include <atomic>
//#include <boost/thread/barrier.hpp>
#include <future>

//http://www.boost.org/doc/libs/1_39_0/doc/html/thread/synchronization.html#thread.synchronization.barriers.barrier

namespace utils
{

template <typename T, int BUFFER_SIZE>
class Pipe
{
public:

    using ProducerFunction = std::function<T(void)>;
    using ConsumerFunction = std::function<void(const T& )>;
    using TypeConstructor = std::function<T(void)>;
    using ObjectCheckFunction = std::function < void(const T&)>;

    Pipe(ProducerFunction prodFunc,
         ConsumerFunction consFunc,
         TypeConstructor constructFunc = nullptr,
         ObjectCheckFunction checkFunc = nullptr)
        : m_prodFunc{prodFunc}
        , m_consFunc{consFunc}
        , m_typeConstructor{constructFunc}
        , m_checkFunc{checkFunc}
        , m_pProducer{}
        , m_pConsumer{}
        , m_bStopPipe{false}
        , m_produceCount{0}
        , m_consumeCount{0}
        , buffer{new T[BUFFER_SIZE]}
    {
        if (!constructFunc)
        {
            m_typeConstructor = [] () { return T(); };
        }

        if (!checkFunc)
        {
            m_checkFunc = [](const T&){};
        }
    }

    auto setTypeConstructor(TypeConstructor constructFunc) -> void
    {
        m_typeConstructor = constructFunc;
    }

    auto setObjectCheckCallback(ObjectCheckFunction checkFunc) -> void
    {
        m_checkFunc = checkFunc;
    }

    ~Pipe()
    {
        stop();
    }

    auto start() -> void
    {
        //check if the functions are set properly.
        if (m_prodFunc == nullptr || m_consFunc == nullptr)
            return;

        /* implementation of a lock free single producer single consumer threads,
        inspired from the following site:
        https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
        */
        auto produceFunc = [this] ()
        {
            while (!m_bStopPipe)
            {
                while (m_produceCount - m_consumeCount == BUFFER_SIZE)
                    std::this_thread::yield();

                auto index = m_produceCount % BUFFER_SIZE;
                m_checkFunc(buffer[index]);
                buffer[index] = m_prodFunc();
                //boost::barrier(1);
                ++m_produceCount;
            }
        };

        auto consumeFunc = [this] ()
        {
            while (!m_bStopPipe)
            {
                while (m_produceCount - m_consumeCount == 0)
                    std::this_thread::yield();

                auto & cur = buffer[m_consumeCount % BUFFER_SIZE];
                m_consFunc(cur);

                //fill the buffer with an empty object as returned by the type constructor.
                //buffer[m_consumeCount % BUFFER_SIZE] = m_typeConstructor();

               // boost::barrier(1);
                ++m_consumeCount;
            }

            //flush the remaining objects before exiting
            while (m_produceCount >= m_consumeCount)
                m_consFunc(buffer[m_consumeCount++ % BUFFER_SIZE]);
        };

        m_bStopPipe = false;
        m_pProducer = std::async(produceFunc);
        m_pConsumer = std::async(consumeFunc);

        // SetThreadPriority(m_pProducerThread->native_handle(), THREAD_PRIORITY_TIME_CRITICAL);

    }

    auto stop() -> void
    {
        m_bStopPipe = true;

        if (m_pProducer.valid())
        {
          m_pProducer.get();
          m_produceCount = 0;
        }

        if (m_pConsumer.valid())
        {
            m_pConsumer.get();
            m_consumeCount = 0;
        }
    }

private:
    TypeConstructor         m_typeConstructor;
    ProducerFunction        m_prodFunc;
    ConsumerFunction        m_consFunc;
    ObjectCheckFunction     m_checkFunc;
    std::future<void>       m_pProducer;
    std::future<void>       m_pConsumer;
    std::atomic<int>        m_produceCount;
    std::atomic<int>        m_consumeCount;
    //boost::barrier          m_barrier;
    bool                    m_bStopPipe;
    std::unique_ptr<T[]>    buffer;
};

}
