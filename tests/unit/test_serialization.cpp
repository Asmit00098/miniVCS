/**
 * tests/unit/test_serialization.cpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Unit tests for object serialization:
 *  - Tree object serialization, parsing, and round-trip identity
 *  - Commit metadata serialization, parsing, and round-trip identity
 *  - Disk-backed commit metadata store (saveCommitMeta / loadCommitMeta)
 */

#include "tests/test_helpers.hpp"
#include "minivcs.hpp"

// ─── Tree Serialization ───────────────────────────────────────────────────────

TEST(SerializationTest, TreeRoundTrip) {
    Tree original;
    original["src/main.cpp"] = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    original["src/utils.hpp"] = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    original["README.md"] = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff21a7f7d799c26ac2cb76";

    std::string serialized = serializeTree(original);
    EXPECT_FALSE(serialized.empty());

    Tree parsed = parseTree(serialized);
    EXPECT_EQ(parsed.size(), original.size());
    EXPECT_EQ(parsed["src/main.cpp"], original["src/main.cpp"]);
    EXPECT_EQ(parsed["src/utils.hpp"], original["src/utils.hpp"]);
    EXPECT_EQ(parsed["README.md"], original["README.md"]);
}

TEST(SerializationTest, EmptyTreeSerialization) {
    Tree emptyTree;
    std::string serialized = serializeTree(emptyTree);
    EXPECT_TRUE(serialized.empty());

    Tree parsed = parseTree(serialized);
    EXPECT_EQ(parsed.size(), 0U);
}

// ─── Commit Metadata Serialization ───────────────────────────────────────────

TEST(SerializationTest, CommitMetaRoundTrip) {
    CommitMeta c;
    c.id = "c1a2b3c4d5e6f708192a3b4c5d6e7f8091a2b3c4";
    c.treeHash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    c.parent = "0102030405060708090a0b0c0d0e0f1011121314";
    c.branch = "main";
    c.author = "Developer <dev@example.com>";
    c.timestamp = "20261012_180000";
    c.message = "Implement Myers diff and content-addressed storage";
    c.extras["gpg_signature"] = "none";

    std::string serialized = serializeCommit(c);
    EXPECT_NE(serialized.find("id=" + c.id), std::string::npos);
    EXPECT_NE(serialized.find("branch=" + c.branch), std::string::npos);
    EXPECT_NE(serialized.find("message=" + c.message), std::string::npos);

    CommitMeta parsed = parseCommit(serialized);
    EXPECT_EQ(parsed.id, c.id);
    EXPECT_EQ(parsed.treeHash, c.treeHash);
    EXPECT_EQ(parsed.parent, c.parent);
    EXPECT_EQ(parsed.branch, c.branch);
    EXPECT_EQ(parsed.author, c.author);
    EXPECT_EQ(parsed.timestamp, c.timestamp);
    EXPECT_EQ(parsed.message, c.message);
    EXPECT_EQ(parsed.extras["gpg_signature"], "none");
}

TEST(SerializationTest, RootCommitHasEmptyParent) {
    CommitMeta root;
    root.id = "root1234567890abcdef1234567890abcdef1234";
    root.treeHash = "tree9876543210fedcba9876543210fedcba9876";
    root.parent = ""; // root commit has no parent
    root.branch = "main";
    root.author = "System";
    root.timestamp = "20261012_000000";
    root.message = "Initial commit";

    std::string serialized = serializeCommit(root);
    CommitMeta parsed = parseCommit(serialized);
    EXPECT_TRUE(parsed.parent.empty());
    EXPECT_EQ(parsed.message, "Initial commit");
}

TEST(SerializationTest, DiskCommitSaveAndLoad) {
    TempRepo repo("commit_disk_io");
    CommitMeta c;
    c.id = "deadbeef1234567890abcdef1234567890abcdef";
    c.treeHash = "feedface1234567890abcdef1234567890abcdef";
    c.parent = "";
    c.branch = "dev";
    c.author = "Alice <alice@test.org>";
    c.timestamp = "20261012_123456";
    c.message = "Test saveCommitMeta and loadCommitMeta";

    saveCommitMeta(c);

    auto loaded = loadCommitMeta(c.id);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->id, c.id);
    EXPECT_EQ(loaded->message, c.message);
    EXPECT_EQ(loaded->branch, c.branch);

    auto missing = loadCommitMeta("nonexistent_commit_hash");
    EXPECT_FALSE(missing.has_value());
}

TEST(SerializationTest, CommitMetaMultiLineMessage) {
    CommitMeta c;
    c.id = "commit_multiline_12345";
    c.treeHash = "tree_hash_12345";
    c.parent = "parent_hash_12345";
    c.branch = "main";
    c.author = "Author <author@example.com>";
    c.timestamp = "20261013_120000";
    c.message = "feat(auth): add OAuth2 provider\n\n- Support Google login\n- Support GitHub login\n- Store token securely";

    std::string serialized = serializeCommit(c);
    CommitMeta parsed = parseCommit(serialized);

    EXPECT_EQ(parsed.id, c.id);
    EXPECT_EQ(parsed.message, c.message);
    EXPECT_EQ(parsed.author, c.author);
}

TEST(SerializationTest, CommitMetaSpecialCharactersAndEscaping) {
    CommitMeta c;
    c.id = "commit_escape_67890";
    c.treeHash = "tree_hash_67890";
    c.parent = "";
    c.branch = "main";
    c.author = "Dev";
    c.timestamp = "20261013_120100";
    c.message = "fix: handle path C:\\Users\\test and key=value pairs with \"quotes\"";

    std::string serialized = serializeCommit(c);
    CommitMeta parsed = parseCommit(serialized);

    EXPECT_EQ(parsed.message, c.message);
}

TEST(SerializationTest, MetaEscapingHelperDirectRoundTrip) {
    std::string original = "Line 1\nLine 2\r\nLine 3\\with\\backslashes=and=equals";
    std::string escaped = escapeMeta(original);
    std::string unescaped = unescapeMeta(escaped);
    EXPECT_EQ(unescaped, original);
}
