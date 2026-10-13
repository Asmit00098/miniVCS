/**
 * tests/unit/test_objects.cpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Unit tests for Content-Addressed Object Store:
 *  - Two-level fan-out object path structure (.miniVCS/objects/<xx>/<rest>)
 *  - Blob storage from file and string
 *  - Object existence check
 *  - Exact content recovery via readObject
 *  - Content deduplication
 */

#include "tests/test_helpers.hpp"
#include "minivcs.hpp"

TEST(ObjectStoreTest, ObjectPathFanoutLayout) {
    std::string testHash = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    std::string path = objectPath(testHash);
    EXPECT_EQ(path, ".miniVCS/objects/ba/7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST(ObjectStoreTest, StoreAndReadBlobString) {
    TempRepo repo("obj_store_str");
    std::string content = "const secret = 'not_a_real_secret';\nfunction test() { return 42; }\n";
    std::string hash = storeBlobStr(content);

    EXPECT_FALSE(hash.empty());
    EXPECT_EQ(hash.length(), 64U);
    EXPECT_TRUE(objectExists(hash));

    std::string recovered = readObject(hash);
    EXPECT_EQ(recovered, content);
}

TEST(ObjectStoreTest, StoreBlobFromFile) {
    TempRepo repo("obj_store_file");
    fs::path src = repo.path() / "source.txt";
    std::string content = "Line 1\nLine 2\nLine 3\n";
    WriteFile(src, content);

    std::string hash = storeBlob(src);
    EXPECT_TRUE(objectExists(hash));
    EXPECT_EQ(readObject(hash), content);
}

TEST(ObjectStoreTest, ContentDeduplication) {
    TempRepo repo("obj_dedup");
    std::string content = "Shared utility code across multiple versions";
    std::string hash1 = storeBlobStr(content);
    std::string hash2 = storeBlobStr(content);

    // Identical content must yield identical hash
    EXPECT_EQ(hash1, hash2);

    // Ensure the blob file exists on disk once
    EXPECT_TRUE(fs::exists(objectPath(hash1)));
}

TEST(ObjectStoreTest, NonExistentObjectReturnsFalse) {
    TempRepo repo("obj_missing");
    std::string fakeHash = "0000000000000000000000000000000000000000000000000000000000000000";
    EXPECT_FALSE(objectExists(fakeHash));
}
