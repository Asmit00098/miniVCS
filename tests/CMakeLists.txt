/*
 * SecureVCS Test Suite — CMakeLists.txt (tests/)
 * ─────────────────────────────────────────────────────────────────────────────
 * Uses Google Test fetched automatically via CMake FetchContent.
 * Tests are split by module:
 *   unit/test_sha256.cpp        — SHA-256 NIST vectors
 *   unit/test_security.cpp      — Secret scanner & filename blocklist
 *   unit/test_diff.cpp          — Myers diff algorithm
 *   unit/test_objects.cpp       — Content-addressed object store
 *   unit/test_serialization.cpp — Tree/commit object serialization
 *   unit/test_config.cpp        — Config & ignore rules
 *   integration/test_cli.cpp    — Full CLI workflow tests
 */

cmake_minimum_required(VERSION 3.20)
project(SecureVCS_Tests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Fetch Google Test ──────────────────────────────────────────────────────────
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ── Source under test ─────────────────────────────────────────────────────────
# We compile main.cpp with MINIVCS_TEST_BUILD so its int main() is excluded.
# All non-main symbols become available to every test target.
set(MINIVCS_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../main.cpp")

# Common compile options
set(SECUREVCS_TEST_FLAGS
    -DMINIVCS_TEST_BUILD
    -DMINIVCS_NO_OPENSSL    # use built-in SHA-256 (no external dep needed for tests)
)

# ── Helper: create a unit test target ─────────────────────────────────────────
function(add_minivcs_test TEST_NAME TEST_SRC)
  add_executable(${TEST_NAME} ${TEST_SRC} ${MINIVCS_SRC})
  target_compile_options(${TEST_NAME} PRIVATE ${SECUREVCS_TEST_FLAGS})
  target_include_directories(${TEST_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/..")
  target_link_libraries(${TEST_NAME} GTest::gtest_main)
  if(WIN32)
    target_link_libraries(${TEST_NAME} ws2_32)
  endif()
  include(GoogleTest)
  gtest_discover_tests(${TEST_NAME})
endfunction()

# ── Register all test targets ─────────────────────────────────────────────────
add_minivcs_test(test_sha256        unit/test_sha256.cpp)
add_minivcs_test(test_security      unit/test_security.cpp)
add_minivcs_test(test_diff          unit/test_diff.cpp)
add_minivcs_test(test_objects       unit/test_objects.cpp)
add_minivcs_test(test_serialization unit/test_serialization.cpp)
add_minivcs_test(test_config        unit/test_config.cpp)
add_minivcs_test(test_ai            unit/test_ai.cpp)
add_minivcs_test(test_cli           integration/test_cli.cpp)
add_minivcs_test(test_sync          integration/test_sync.cpp)
add_minivcs_test(test_ai_live       integration/test_ai_live.cpp)
