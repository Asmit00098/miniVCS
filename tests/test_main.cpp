/**
 * tests/test_main.cpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Master test runner entry point for SecureVCS.
 * Used for standalone builds and unified test runs.
 */

#include "tests/test_helpers.hpp"

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
