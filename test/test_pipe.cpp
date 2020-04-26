///
/// @file test_timer.cpp
///
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <gtest/gtest.h>
#include "pipe.h"

class PipeShould : public ::testing::Test
{
protected:
    std::unique_ptr<Pipe> pipe_;

    void SetUp(uint32_t tick_time)
    {
        pipe_ = std::make_unique<Pipe>();
    }
};


int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
