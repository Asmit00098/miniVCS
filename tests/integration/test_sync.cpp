/**
 * tests/integration/test_sync.cpp
 * ─────────────────────────────────────────────────────────────────────────────
 * End-to-end integration tests for Remote Sync:
 *  - Push commits, blobs, and branch references to remote storage backend
 *  - Pull remote commits into a fresh or existing repository
 *  - Clone complete repository history from remote storage
 *  - Validate data integrity across backup / recovery cycles
 */

#include "tests/test_helpers.hpp"
#include "minivcs.hpp"

TEST(SyncIntegrationTest, PushAndCloneFullRepositoryCycle) {
    TempRepo testEnv("sync_cycle");

    // 1. Setup a dedicated local remote storage directory
    fs::path remoteStorage = testEnv.path() / "central_storage";
    fs::create_directories(remoteStorage);

    // 2. Setup Source Repository
    fs::path sourceDir = testEnv.path() / "source_repo";
    fs::create_directories(sourceDir);
    fs::current_path(sourceDir);

    {
        // Init with user "alice", backend 7 (local), bucket path pointing to remoteStorage
        FakeStdin input("alice\n7\n" + remoteStorage.string() + "\n");
        CaptureStdout cap;
        cmd_init();
    }
    ASSERT_TRUE(repoInitialized());

    // Create file and commit
    WriteFile(sourceDir / "src" / "main.py", "print('SecureVCS v1')\n");
    WriteFile(sourceDir / "README.md", "# Distributed Project\n");
    cmd_add("src/main.py");
    cmd_add("README.md");
    dispatchCommit({"-m", "Initial commit from Alice"});

    std::string commit1 = headCommitId();
    ASSERT_FALSE(commit1.empty());

    // Second commit
    WriteFile(sourceDir / "src" / "main.py", "print('SecureVCS v2 - Cloud Sync')\n");
    cmd_add("src/main.py");
    cmd_add("README.md");
    dispatchCommit({"-m", "Feature upgrade in v2"});

    std::string commit2 = headCommitId();
    ASSERT_NE(commit1, commit2);

    // Explicit push to remote
    {
        CaptureStdout cap;
        cmd_push();
        EXPECT_OUTPUT_CONTAINS(cap, "Push complete.");
    }

    // Verify remote storage received objects, commits, and refs
    EXPECT_TRUE(fs::exists(remoteStorage / "alice" / "objects"));
    EXPECT_TRUE(fs::exists(remoteStorage / "alice" / "commits" / commit1));
    EXPECT_TRUE(fs::exists(remoteStorage / "alice" / "commits" / commit2));
    EXPECT_TRUE(fs::exists(remoteStorage / "alice" / "refs" / "heads" / "main"));

    // 3. Clone into Destination Directory
    fs::path cloneDir = testEnv.path() / "cloned_repo";
    {
        CaptureStdout cap;
        cmd_clone("alice", cloneDir.string());
        EXPECT_OUTPUT_CONTAINS(cap, "Cloned 'alice'");
    }

    // 4. Verify cloned directory contents and commit history
    EXPECT_TRUE(fs::exists(cloneDir / ".miniVCS"));
    EXPECT_TRUE(fs::exists(cloneDir / "src" / "main.py"));
    EXPECT_TRUE(fs::exists(cloneDir / "README.md"));
    EXPECT_FILE_CONTAINS(cloneDir / "src" / "main.py", "SecureVCS v2 - Cloud Sync");
    EXPECT_FILE_CONTAINS(cloneDir / "README.md", "Distributed Project");

    // Switch CWD to cloned repo and verify git-style commands
    fs::current_path(cloneDir);
    ASSERT_TRUE(repoInitialized());
    EXPECT_EQ(headCommitId(), commit2);

    auto meta = loadCommitMeta(commit2);
    ASSERT_TRUE(meta.has_value());
    EXPECT_EQ(meta->message, "Feature upgrade in v2");
    EXPECT_EQ(meta->parent, commit1);

    // Recover commit 1 via checkout
    {
        CaptureStdout cap;
        cmd_checkout(commit1);
        EXPECT_OUTPUT_CONTAINS(cap, "Checked out");
    }
    EXPECT_FILE_CONTAINS(cloneDir / "src" / "main.py", "SecureVCS v1");
}
