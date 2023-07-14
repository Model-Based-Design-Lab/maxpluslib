#pragma once

namespace testing {

#define ASSERT_THROW(condition)                                                                    \
    {                                                                                              \
        if (!(condition)) {                                                                        \
            throw std::runtime_error(std::string("Assertion violated.") + std::string("\nIn:")     \
                                     + std::string(__FILE__) + std::string(":")                    \
                                     + std::to_string(__LINE__) + std::string(" in ")              \
                                     + std::string(__FUNCTION__));                                 \
        }                                                                                          \
    }

#define ASSERT_EQUAL(x, y)                                                                         \
    {                                                                                              \
        if ((x) != (y)) {                                                                          \
            throw std::runtime_error(                                                              \
                    std::string("Asserted equality violated.") + std::string("\nIn:")              \
                    + std::string(__FILE__) + std::string(":") + std::to_string(__LINE__)          \
                    + std::string(" in ") + std::string(__FUNCTION__) + std::string(": ")          \
                    + std::to_string((x)) + std::string(" != ") + std::to_string((y)));            \
        }                                                                                          \
    }

#define ASSERT_DBL_EPSILON 1e-8

#define ASSERT_DOUBLE_EQUAL(x, y)                                                                  \
    {                                                                                              \
        if (((x) < (y - ASSERT_DBL_EPSILON)) || ((y) < (x - ASSERT_DBL_EPSILON))) {                \
            throw std::runtime_error(                                                              \
                    std::string("Asserted double equality violated.") + std::string("\nIn:")       \
                    + std::string(__FILE__) + std::string(":") + std::to_string(__LINE__)          \
                    + std::string(" in ") + std::string(__FUNCTION__) + std::string(": ")          \
                    + std::to_string((x)) + std::string(" != ") + std::to_string((y)));            \
        }                                                                                          \
    }

class Test {

public:
    virtual void SetUp() = 0;
    virtual void TearDown() = 0;
    virtual void Run() = 0;
};

} // namespace testing
