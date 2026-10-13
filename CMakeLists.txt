cmake_minimum_required(VERSION 3.20)
project(SecureVCS CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Main binary ───────────────────────────────────────────────────────────────
add_executable(miniVCS main.cpp)
target_compile_options(miniVCS PRIVATE -DMINIVCS_NO_OPENSSL)
if(WIN32)
    target_link_libraries(miniVCS ws2_32)
endif()

# ── Tests (optional, enable with -DBUILD_TESTS=ON) ───────────────────────────
option(BUILD_TESTS "Build the test suite" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
