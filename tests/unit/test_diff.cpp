/**
 * tests/unit/test_diff.cpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Unit tests for Myers line-level diff algorithm & patch serialization:
 *  - Identical files (no delta)
 *  - Empty file comparisons
 *  - Insertions and deletions
 *  - Modification sequences
 *  - Patch serialization syntax
 */

#include "tests/test_helpers.hpp"
#include "minivcs.hpp"

TEST(DiffTest, IdenticalInputsProduceNoChanges) {
    std::vector<std::string> A = {"line 1", "line 2", "line 3"};
    std::vector<std::string> B = {"line 1", "line 2", "line 3"};

    auto ops = myersDiff(A, B);
    EXPECT_EQ(ops.size(), 3U);
    for (const auto& op : ops) {
        EXPECT_EQ(op.op, '=');
    }

    std::string patch = serializePatch(ops);
    EXPECT_TRUE(patch.empty());
}

TEST(DiffTest, BothEmptyInputs) {
    std::vector<std::string> A = {};
    std::vector<std::string> B = {};

    auto ops = myersDiff(A, B);
    EXPECT_EQ(ops.size(), 0U);
    EXPECT_TRUE(serializePatch(ops).empty());
}

TEST(DiffTest, AdditionToEmptyFile) {
    std::vector<std::string> A = {};
    std::vector<std::string> B = {"hello", "world"};

    auto ops = myersDiff(A, B);
    EXPECT_EQ(ops.size(), 2U);
    for (const auto& op : ops) {
        EXPECT_EQ(op.op, '+');
    }

    std::string patch = serializePatch(ops);
    EXPECT_NE(patch.find("+0 hello"), std::string::npos);
    EXPECT_NE(patch.find("+1 world"), std::string::npos);
}

TEST(DiffTest, DeletionToEmptyFile) {
    std::vector<std::string> A = {"delete me", "delete me too"};
    std::vector<std::string> B = {};

    auto ops = myersDiff(A, B);
    EXPECT_EQ(ops.size(), 2U);
    for (const auto& op : ops) {
        EXPECT_EQ(op.op, '-');
    }

    std::string patch = serializePatch(ops);
    EXPECT_NE(patch.find("-0 delete me"), std::string::npos);
    EXPECT_NE(patch.find("-1 delete me too"), std::string::npos);
}

TEST(DiffTest, MiddleLineModification) {
    std::vector<std::string> A = {"header", "old line", "footer"};
    std::vector<std::string> B = {"header", "new line", "footer"};

    auto ops = myersDiff(A, B);
    bool sawDel = false, sawAdd = false, sawHeader = false, sawFooter = false;

    for (const auto& op : ops) {
        if (op.op == '=' && op.line == "header") sawHeader = true;
        if (op.op == '=' && op.line == "footer") sawFooter = true;
        if (op.op == '-' && op.line == "old line") sawDel = true;
        if (op.op == '+' && op.line == "new line") sawAdd = true;
    }

    EXPECT_TRUE(sawHeader);
    EXPECT_TRUE(sawFooter);
    EXPECT_TRUE(sawDel);
    EXPECT_TRUE(sawAdd);

    std::string patch = serializePatch(ops);
    EXPECT_NE(patch.find("-1 old line"), std::string::npos);
    EXPECT_NE(patch.find("+1 new line"), std::string::npos);
}
