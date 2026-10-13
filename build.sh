#!/bin/bash
# Build SecureVCS C++ binary for all platforms
# Usage: ./build.sh [mac|win|linux|all]

TARGET=${1:-all}

build_mac() {
    echo "Building for macOS..."
    mkdir -p bin/mac
    # Try with OpenSSL first
    if command -v brew &>/dev/null && [ -d "$(brew --prefix openssl 2>/dev/null)" ]; then
        OSSL=$(brew --prefix openssl)
        g++ -std=c++17 -O2 main.cpp \
            -L"$OSSL/lib" -I"$OSSL/include" \
            -lssl -lcrypto \
            -o bin/mac/miniVCS && \
        echo "[OK] macOS binary (with OpenSSL) → bin/mac/miniVCS"
    else
        g++ -std=c++17 -O2 -DMINIVCS_NO_OPENSSL main.cpp \
            -o bin/mac/miniVCS && \
        echo "[OK] macOS binary (built-in SHA-256) → bin/mac/miniVCS"
    fi
}

build_linux() {
    echo "Building for Linux..."
    mkdir -p bin/linux
    g++ -std=c++17 -O2 main.cpp -lssl -lcrypto -o bin/linux/miniVCS && \
    echo "[OK] Linux binary → bin/linux/miniVCS"
}

build_windows() {
    echo "Building for Windows (cross-compile)..."
    mkdir -p bin/windows
    if command -v x86_64-w64-mingw32-g++ &>/dev/null; then
        x86_64-w64-mingw32-g++ -std=c++17 -O2 -DMINIVCS_NO_OPENSSL \
            main.cpp -o bin/windows/miniVCS.exe -lws2_32 && \
        echo "[OK] Windows binary → bin/windows/miniVCS.exe"
    else
        echo "[!] mingw cross-compiler not found. Skipping Windows build."
        echo "    On Windows: g++ -std=c++17 -O2 -DMINIVCS_NO_OPENSSL main.cpp -o miniVCS.exe"
    fi
}

case "$TARGET" in
    mac)     build_mac ;;
    linux)   build_linux ;;
    win)     build_windows ;;
    test)    ./run_tests.sh ;;
    all)     build_mac; build_linux; build_windows ;;
    *)       echo "Usage: ./build.sh [mac|linux|win|test|all]"; exit 1 ;;
esac