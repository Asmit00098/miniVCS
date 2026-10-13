/**
 * tests/unit/test_sha256.cpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Unit tests for SHA-256 implementation:
 *  - Standard NIST CAVP test vectors
 *  - Multi-block inputs (> 55 and > 64 bytes)
 *  - Avalanche effect / determinism
 *  - sha256File vs sha256Str equivalence
 *  - Non-existent file error handling
 */

#include "tests/test_helpers.hpp"
#include "minivcs.hpp"

TEST(Sha256Test, EmptyStringNistVector) {
    // NIST CAVP: SHA-256 of empty message
    const std::string expected = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    std::string actual = sha256Str("");
    EXPECT_EQ(actual, expected);
    EXPECT_EQ(actual.length(), 64U);
}

TEST(Sha256Test, ShortStringNistVector) {
    // NIST CAVP: SHA-256 of "abc"
    const std::string expected = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    std::string actual = sha256Str("abc");
    EXPECT_EQ(actual, expected);
}

TEST(Sha256Test, MultiBlockNistVector) {
    // NIST CAVP: 56-byte message crossing the 55-byte single-block threshold
    const std::string input = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    const std::string expected = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";
    std::string actual = sha256Str(input);
    EXPECT_EQ(actual, expected);
}

TEST(Sha256Test, Determinism) {
    std::string sample = "SecureVCS - Cloud-Native Version Control System 2026";
    std::string h1 = sha256Str(sample);
    std::string h2 = sha256Str(sample);
    EXPECT_EQ(h1, h2);
}

TEST(Sha256Test, AvalancheEffect) {
    // 1-character difference should yield completely different hash
    std::string h1 = sha256Str("The quick brown fox jumps over the lazy dog");
    std::string h2 = sha256Str("The quick brown fox jumps over the lazy cog");
    EXPECT_NE(h1, h2);
    EXPECT_EQ(h1.length(), 64U);
    EXPECT_EQ(h2.length(), 64U);
}

TEST(Sha256Test, FileAndStringEquivalence) {
    TempRepo repo("sha256_file");
    const std::string content = "Hello, SecureVCS content-addressed object store!\n";
    fs::path testFile = repo.path() / "test.txt";
    WriteFile(testFile, content);

    std::string strHash = sha256Str(content);
    std::string fileHash = sha256File(testFile);

    EXPECT_EQ(fileHash, strHash);
    EXPECT_FALSE(fileHash.empty());
}

TEST(Sha256Test, NonExistentFileReturnsEmpty) {
    TempRepo repo("sha256_missing");
    fs::path missing = repo.path() / "does_not_exist_xyz.bin";
    std::string h = sha256File(missing);
    EXPECT_TRUE(h.empty());
}
