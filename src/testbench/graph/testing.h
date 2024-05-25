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

#define ASSERT(x)                                                                                  \
    {                                                                                              \
        if (!(x)) {                                                                                \
            throw std::runtime_error(std::string("Assert condition violated.")                     \
                                     + std::string("\nIn:") + std::string(__FILE__)                \
                                     + std::string(":") + std::to_string(__LINE__)                 \
                                     + std::string(" in ") + std::string(__FUNCTION__)             \
                                     + std::string(": ") + std::to_string((x)));                   \
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

#define ASSERT_APPROX_EQUAL(x, y, eps)                                                             \
    {                                                                                              \
        if (std::abs((x) - (y)) > eps) {                                                           \
            throw std::runtime_error(                                                              \
                    std::string("Asserted approximate equality violated.") + std::string("\nIn:")  \
                    + std::string(__FILE__) + std::string(":") + std::to_string(__LINE__)          \
                    + std::string(" in ") + std::string(__FUNCTION__) + std::string(": ")          \
                    + std::to_string((x)) + std::string(" != ") + std::to_string((y)));            \
        }                                                                                          \
    }

#define ASSERT_EQUAL_NOPRINT(x, y)                                                                 \
    {                                                                                              \
        if ((x) != (y)) {                                                                          \
            throw std::runtime_error(std::string("Asserted equality violated.")                    \
                                     + std::string("\nIn:") + std::string(__FILE__)                \
                                     + std::string(":") + std::to_string(__LINE__)                 \
                                     + std::string(" in ") + std::string(__FUNCTION__));           \
        }                                                                                          \
    }

class Test {

public:
    virtual void SetUp() = 0;
    virtual void TearDown() = 0;
    virtual void Run() = 0;
};

} // namespace testing
