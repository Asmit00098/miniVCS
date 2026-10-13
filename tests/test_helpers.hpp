/**
 * tests/test_helpers.hpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Shared test infrastructure for the SecureVCS test suite.
 *
 * Provides:
 *   TempRepo  — RAII scoped temporary directory with repo isolation
 *   RunResult — structured result of capturing stdout/stderr
 *   CaptureStdout — redirect cout for unit tests
 *   WriteFile / ReadFile — helpers for filesystem assertions
 *   EXPECT_FILE_EXISTS / EXPECT_FILE_CONTAINS — custom assertion macros
 */

#pragma once

#if __has_include(<gtest/gtest.h>)
#include <gtest/gtest.h>
#else
#include "mini_gtest.hpp"
#endif
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <functional>
#include <cstdlib>

namespace fs = std::filesystem;

// ─── TempRepo ─────────────────────────────────────────────────────────────────
// RAII class that:
//   1. Creates a unique temp directory
//   2. Changes the process working directory into it
//   3. On destruction: restores the original CWD and deletes the temp dir

class TempRepo {
public:
    explicit TempRepo(const std::string& name = "test_repo") {
        original_cwd_ = fs::current_path();
        // Place temp dirs inside the build directory so they don't litter the source
        temp_dir_ = fs::temp_directory_path() /
                    ("minivcs_test_" + name + "_" + std::to_string(
                        std::chrono::system_clock::now().time_since_epoch().count()));
        fs::create_directories(temp_dir_);
        fs::current_path(temp_dir_);
    }

    ~TempRepo() {
        try {
            fs::current_path(original_cwd_);
            fs::remove_all(temp_dir_);
        } catch (...) {}
    }

    // Path to the temp directory
    fs::path path() const { return temp_dir_; }

    // Current working directory (same as path() after construction)
    fs::path cwd() const { return fs::current_path(); }

    // Prevent copy
    TempRepo(const TempRepo&) = delete;
    TempRepo& operator=(const TempRepo&) = delete;

private:
    fs::path temp_dir_;
    fs::path original_cwd_;
};

// ─── CaptureStdout ────────────────────────────────────────────────────────────
// Redirects std::cout to a string buffer for the duration of a scope.
// Usage:
//   CaptureStdout cap;
//   some_function_that_prints();
//   EXPECT_THAT(cap.str(), HasSubstr("OK"));

class CaptureStdout {
public:
    CaptureStdout() : old_buf_(std::cout.rdbuf(buf_.rdbuf())) {}
    ~CaptureStdout() { std::cout.rdbuf(old_buf_); }

    std::string str() const { return buf_.str(); }
    void clear() { buf_.str(""); buf_.clear(); }

private:
    std::ostringstream buf_;
    std::streambuf*    old_buf_;
};

// ─── File helpers ─────────────────────────────────────────────────────────────

inline void WriteFile(const fs::path& path, const std::string& content) {
    fs::create_directories(path.parent_path());
    std::ofstream f(path, std::ios::binary);
    ASSERT_TRUE(f.is_open()) << "Could not open for writing: " << path;
    f << content;
}

inline std::string ReadFile(const fs::path& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return "";
    return std::string(std::istreambuf_iterator<char>(f),
                       std::istreambuf_iterator<char>());
}

inline bool FileContains(const fs::path& path, const std::string& needle) {
    return ReadFile(path).find(needle) != std::string::npos;
}

// ─── Custom assertion macros ──────────────────────────────────────────────────

#define EXPECT_FILE_EXISTS(path) \
    EXPECT_TRUE(fs::exists(path)) << "Expected file to exist: " << (path)

#define EXPECT_FILE_NOT_EXISTS(path) \
    EXPECT_FALSE(fs::exists(path)) << "Expected file NOT to exist: " << (path)

#define EXPECT_FILE_CONTAINS(path, needle) \
    EXPECT_TRUE(FileContains(path, needle)) \
        << "Expected file " << (path) << " to contain: " << (needle) \
        << "\nActual content:\n" << ReadFile(path)

#define ASSERT_FILE_EXISTS(path) \
    ASSERT_TRUE(fs::exists(path)) << "Required file missing: " << (path)

// ─── Stdout assertion helpers ─────────────────────────────────────────────────

#define EXPECT_OUTPUT_CONTAINS(capture, substr) \
    EXPECT_NE(std::string::npos, (capture).str().find(substr)) \
        << "Expected stdout to contain: \"" << (substr) << "\"\n" \
        << "Actual stdout:\n" << (capture).str()

#define EXPECT_OUTPUT_NOT_CONTAINS(capture, substr) \
    EXPECT_EQ(std::string::npos, (capture).str().find(substr)) \
        << "Expected stdout NOT to contain: \"" << (substr) << "\"\n" \
        << "Actual stdout:\n" << (capture).str()

// ─── Fake stdin helper ────────────────────────────────────────────────────────
// Redirect std::cin from a string for functions that prompt the user.

class FakeStdin {
public:
    explicit FakeStdin(const std::string& input)
        : buf_(input), old_buf_(std::cin.rdbuf(buf_.rdbuf())) {
        std::cin.clear();
    }
    ~FakeStdin() {
        std::cin.rdbuf(old_buf_);
        std::cin.clear();
    }

private:
    std::istringstream buf_;
    std::streambuf*    old_buf_;
};

// ─── splitLines helper ────────────────────────────────────────────────────────
inline std::vector<std::string> splitLines(const std::string& s) {
    std::vector<std::string> lines;
    std::istringstream ss(s);
    std::string line;
    while (std::getline(ss, line)) lines.push_back(line);
    return lines;
}
