#!/usr/bin/env bash
# SecureVCS Test Runner (Linux / macOS / Git Bash)
# Usage:
#   ./run_tests.sh          # Build and run all tests
#   ./run_tests.sh clean    # Clean build

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$DIR"

mkdir -p bin

OUT="bin/test_runner"
if [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "cygwin"* || "$OSTYPE" == "win32"* ]]; then
    OUT="bin/test_runner.exe"
fi

NEED_BUILD=0
if [[ "$1" == "clean" || ! -f "$OUT" ]]; then
    NEED_BUILD=1
else
    for src in main.cpp minivcs.hpp tests/**/*.cpp tests/**/*.hpp; do
        if [[ -f "$src" && "$src" -nt "$OUT" ]]; then
            NEED_BUILD=1
            break
        fi
    done
fi

if [[ $NEED_BUILD -eq 1 ]]; then
    echo -e "\033[33mCompiling test suite (C++17)...\033[0m"
    EXTRA_FLAGS=""
    if [[ "$OSTYPE" == "msys"* || "$OSTYPE" == "cygwin"* || "$OSTYPE" == "win32"* ]]; then
        EXTRA_FLAGS="-lws2_32"
    fi

    g++ -std=c++17 -O2 -I. \
        -DMINIVCS_TEST_BUILD \
        -DMINIVCS_NO_OPENSSL \
        main.cpp \
        tests/test_main.cpp \
        tests/unit/test_sha256.cpp \
        tests/unit/test_security.cpp \
        tests/unit/test_diff.cpp \
        tests/unit/test_objects.cpp \
        tests/unit/test_serialization.cpp \
        tests/unit/test_config.cpp \
        tests/integration/test_cli.cpp \
        $EXTRA_FLAGS \
        -o "$OUT"

    echo -e "\033[32mBuild successful.\033[0m\n"
fi

echo -e "\033[36mRunning SecureVCS Test Suite...\033[0m"
./"$OUT"
