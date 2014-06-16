#include "log_pattern.hpp"
#include "gtest/gtest.h"
#include <queue>

TEST(log_pattern, parsing)
{
    log_pattern a("log.12.13");
    ASSERT_EQ(a.boot_count, (size_t)12);
    ASSERT_EQ(a.sequence_number, (size_t)13);
}

TEST(log_pattern, seq)
{
    log_pattern a("log.0.1");
    log_pattern b("log.0.2");
    ASSERT_TRUE(a < b);
}

TEST(log_pattern, queue)
{
    log_pattern a("log.1.12");
    log_pattern b("log.2.23");
    std::priority_queue<log_pattern,
        std::vector<log_pattern>,
        std::greater<log_pattern>> q;
    q.push(a);
    q.push(b);
    ASSERT_EQ(q.top(), a);


}

TEST(log_pattern, queue2)
{
    std::priority_queue<log_pattern,
        std::vector<log_pattern>,
        std::greater<log_pattern>> q;

    for(size_t s = 0; s < 100; ++s)
    {
        for(size_t b = 0; b < 100; ++b)
        {
            std::stringstream ss;
            ss << "log." << b << "." << s;
            q.emplace(ss.str());
        }
    }

    ASSERT_EQ(q.top().boot_count, (size_t)0);
    ASSERT_EQ(q.top().sequence_number, (size_t)0);


}

