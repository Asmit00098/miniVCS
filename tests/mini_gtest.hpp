/**
 * tests/mini_gtest.hpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Zero-dependency, Google Test-compatible micro test framework.
 *
 * Implements the core Google Test API (TEST, TEST_F, EXPECT_*, ASSERT_*,
 * InitGoogleTest, RUN_ALL_TESTS, HasSubstr) with zero external dependencies.
 * Automatically used when <gtest/gtest.h> is not available in the environment.
 */

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <functional>
#include <chrono>
#include <memory>
#include <cstdlib>

namespace testing {

class Test {
public:
    virtual ~Test() = default;
    virtual void SetUp() {}
    virtual void TearDown() {}
    virtual void TestBody() = 0;
};

struct TestInfo {
    std::string suite_name;
    std::string test_name;
    std::function<std::unique_ptr<Test>()> factory;
};

class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry reg;
        return reg;
    }

    void registerTest(const std::string& suite, const std::string& name,
                      std::function<std::unique_ptr<Test>()> factory) {
        tests_.push_back({suite, name, std::move(factory)});
    }

    const std::vector<TestInfo>& tests() const { return tests_; }

    int current_test_failures = 0;
    std::vector<std::string> failure_messages;

private:
    std::vector<TestInfo> tests_;
};

inline void InitGoogleTest(int* /*argc*/, char** /*argv*/) {
    // No-op for command-line parsing in micro runner
}

class AssertionResult {
public:
    AssertionResult(bool success) : success_(success) {}

    operator bool() const { return success_; }

    template <typename T>
    AssertionResult& operator<<(const T& msg) {
        if (!success_) {
            stream_ << msg;
        }
        return *this;
    }

    std::string message() const { return stream_.str(); }

private:
    bool success_;
    std::ostringstream stream_;
};

template <typename T, typename = void>
struct is_streamable : std::false_type {};

template <typename T>
struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>> : std::true_type {};

template <typename T>
std::string formatVal(const T& val) {
    if constexpr (is_streamable<T>::value) {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    } else {
        return "<unprintable>";
    }
}

struct TestAbortException {};

class AssertionHelper {
public:
    AssertionHelper(bool fatal, const char* file, int line, const char* expr)
        : fatal_(fatal), file_(file), line_(line), expr_(expr) {}

    template <typename T>
    AssertionHelper& operator<<(const T& val) {
        if constexpr (is_streamable<T>::value) {
            msg_ << val;
        } else {
            msg_ << "<unprintable>";
        }
        return *this;
    }

    ~AssertionHelper() noexcept(false) {
        std::ostringstream oss;
        oss << file_ << ":" << line_ << ": " << (fatal_ ? "Fatal Failure" : "Failure")
            << "\nValue of: " << expr_ << "\n" << msg_.str();
        TestRegistry::instance().current_test_failures++;
        TestRegistry::instance().failure_messages.push_back(oss.str());
        if (fatal_ && std::uncaught_exceptions() == 0) {
            throw TestAbortException();
        }
    }

private:
    bool fatal_;
    const char* file_;
    int line_;
    const char* expr_;
    std::ostringstream msg_;
};

// HasSubstr matcher helper
inline auto HasSubstr(const std::string& needle) {
    return [needle](const std::string& haystack) -> bool {
        return haystack.find(needle) != std::string::npos;
    };
}

inline int RunAllTests() {
    auto& reg = TestRegistry::instance();
    int passed = 0;
    int failed = 0;
    const auto& tests = reg.tests();

    std::cout << "\033[32m[==========]\033[0m Running " << tests.size()
              << " test(s) from test suite.\n";

    auto start_all = std::chrono::high_resolution_clock::now();

    for (const auto& ti : tests) {
        reg.current_test_failures = 0;
        reg.failure_messages.clear();

        std::cout << "\033[32m[ RUN      ]\033[0m "
                  << ti.suite_name << "." << ti.test_name << "\n";

        auto start = std::chrono::high_resolution_clock::now();
        std::unique_ptr<Test> test_instance = ti.factory();

        try {
            test_instance->SetUp();
            test_instance->TestBody();
            test_instance->TearDown();
        } catch (const TestAbortException&) {
            // Test aborted cleanly on fatal ASSERT
        } catch (const std::exception& ex) {
            reg.current_test_failures++;
            reg.failure_messages.push_back(std::string("Unhandled exception: ") + ex.what());
        } catch (...) {
            reg.current_test_failures++;
            reg.failure_messages.push_back("Unhandled unknown exception");
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        if (reg.current_test_failures == 0) {
            std::cout << "\033[32m[       OK ]\033[0m "
                      << ti.suite_name << "." << ti.test_name
                      << " (" << duration_ms << " ms)\n";
            passed++;
        } else {
            for (const auto& fm : reg.failure_messages) {
                std::cout << fm << "\n";
            }
            std::cout << "\033[31m[  FAILED  ]\033[0m "
                      << ti.suite_name << "." << ti.test_name
                      << " (" << duration_ms << " ms)\n";
            failed++;
        }
    }

    auto end_all = std::chrono::high_resolution_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_all - start_all).count();

    std::cout << "\033[32m[==========]\033[0m " << tests.size()
              << " test(s) ran (" << total_ms << " ms total).\n";
    std::cout << "\033[32m[  PASSED  ]\033[0m " << passed << " test(s).\n";

    if (failed > 0) {
        std::cout << "\033[31m[  FAILED  ]\033[0m " << failed << " test(s).\n";
        return 1;
    }
    return 0;
}

} // namespace testing

#define RUN_ALL_TESTS() ::testing::RunAllTests()

#define TEST(suite_name, test_name) \
    class suite_name##_##test_name##_Test : public ::testing::Test { \
    public: \
        void TestBody() override; \
    }; \
    static const int suite_name##_##test_name##_reg = []() { \
        ::testing::TestRegistry::instance().registerTest(#suite_name, #test_name, []() { \
            return std::make_unique<suite_name##_##test_name##_Test>(); \
        }); \
        return 0; \
    }(); \
    void suite_name##_##test_name##_Test::TestBody()

#define TEST_F(fixture_name, test_name) \
    class fixture_name##_##test_name##_Test : public fixture_name { \
    public: \
        void TestBody() override; \
    }; \
    static const int fixture_name##_##test_name##_reg = []() { \
        ::testing::TestRegistry::instance().registerTest(#fixture_name, #test_name, []() { \
            return std::make_unique<fixture_name##_##test_name##_Test>(); \
        }); \
        return 0; \
    }(); \
    void fixture_name##_##test_name##_Test::TestBody()

#define EXPECT_TRUE(cond) \
    if (!(cond)) ::testing::AssertionHelper(false, __FILE__, __LINE__, #cond)

#define EXPECT_FALSE(cond) \
    if (cond) ::testing::AssertionHelper(false, __FILE__, __LINE__, "!(" #cond ")")

#define EXPECT_EQ(a, b) \
    if (!((a) == (b))) ::testing::AssertionHelper(false, __FILE__, __LINE__, #a " == " #b) \
        << "  Actual:   " << ::testing::formatVal(a) << "\n  Expected: " << ::testing::formatVal(b) << "\n"

#define EXPECT_NE(a, b) \
    if (!((a) != (b))) ::testing::AssertionHelper(false, __FILE__, __LINE__, #a " != " #b) \
        << "  Actual: " << ::testing::formatVal(a) << "\n"

#define ASSERT_TRUE(cond) \
    if (!(cond)) ::testing::AssertionHelper(true, __FILE__, __LINE__, #cond)

#define ASSERT_FALSE(cond) \
    if (cond) ::testing::AssertionHelper(true, __FILE__, __LINE__, "!(" #cond ")")

#define ASSERT_EQ(a, b) \
    if (!((a) == (b))) ::testing::AssertionHelper(true, __FILE__, __LINE__, #a " == " #b) \
        << "  Actual:   " << ::testing::formatVal(a) << "\n  Expected: " << ::testing::formatVal(b) << "\n"

#define ASSERT_NE(a, b) \
    if (!((a) != (b))) ::testing::AssertionHelper(true, __FILE__, __LINE__, #a " != " #b) \
        << "  Actual: " << ::testing::formatVal(a) << "\n"

#define EXPECT_THAT(val, matcher) \
    if (!matcher(val)) ::testing::AssertionHelper(false, __FILE__, __LINE__, #val " matches " #matcher)
