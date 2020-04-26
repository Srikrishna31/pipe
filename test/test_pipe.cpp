///
/// @file test_timer.cpp
///
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <gtest/gtest.h>
#include "pipe.h"


template<typename T>
class SingleMemoryLocationPipeShould : public ::testing::Test
{
protected:
    using SingleMemoryLocationPipe = utils::Pipe<T, 1>;

    std::unique_ptr<SingleMemoryLocationPipe> pipe_;

    void SetUp(typename SingleMemoryLocationPipe::ProducerFunction prod, typename SingleMemoryLocationPipe::ConsumerFunction cons)
    {
        pipe_ = std::make_unique<SingleMemoryLocationPipe>(prod, cons);
    }
};

using TestTypes =  ::testing::Types<int>;
TYPED_TEST_SUITE(SingleMemoryLocationPipeShould, TestTypes);

TYPED_TEST(SingleMemoryLocationPipeShould, CorrectlyWorkForTwoThreadsWithASingleMemoryLocation)
{
    int i = 0;
    const int STOP = 10;
    auto cv = std::condition_variable{};
    auto prod = [&i, &STOP]() {
        for (int j = 0; j < STOP; j++)
            i = j;
        return i;
    };

    auto cons = [&i, &cv, &STOP] (const int j) {
        ASSERT_EQ(i, j);
        if (i == STOP) {
            cv.notify_one();
        }
    };

    SetUp(prod, cons);

    pipe_->start();

    auto lock = std::mutex{};
    auto ulk = std::unique_lock{lock};
    cv.wait(ulk);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
