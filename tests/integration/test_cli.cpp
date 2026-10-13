/**
 * tests/integration/test_cli.cpp
 * ─────────────────────────────────────────────────────────────────────────────
 * End-to-end integration tests for SecureVCS CLI workflows:
 *  - Repository initialization (cmd_init)
 *  - Security guardrail blocking & force override (cmd_add)
 *  - Staging index updates and status check (cmd_status)
 *  - Commit creation, tree storage, and HEAD update (cmd_commit)
 *  - Branch creation and switching (cmd_branch)
 *  - Reset staged changes (cmd_reset)
 *  - Full checkout and version recovery (cmd_checkout)
 */

#include "tests/test_helpers.hpp"
#include "minivcs.hpp"

// Helper to initialize an isolated test repo using local storage backend (no AWS network calls)
inline void InitTestRepo(const std::string& name = "tester") {
    FakeStdin input(name + "\n7\n.local_remote\n");
    CaptureStdout cap;
    cmd_init();
}

TEST(CliIntegrationTest, InitCreatesRepositoryStructure) {
    TempRepo repo("cli_init");

    EXPECT_FALSE(repoInitialized());

    {
        FakeStdin input("alice\n7\n.alice_remote\n");
        CaptureStdout cap;
        cmd_init();
    }

    EXPECT_TRUE(repoInitialized());
    EXPECT_FILE_EXISTS(".miniVCS/HEAD");
    EXPECT_FILE_EXISTS(".miniVCS/config");
    EXPECT_FILE_EXISTS(".miniVCSignore");
    EXPECT_TRUE(fs::is_directory(".miniVCS/objects"));
    EXPECT_TRUE(fs::is_directory(".miniVCS/commits"));
    EXPECT_TRUE(fs::is_directory(".miniVCS/refs/heads"));
    EXPECT_EQ(currentBranch(), "main");
}

TEST(CliIntegrationTest, AddBlocksSecretFilesByDefault) {
    TempRepo repo("cli_add_security");
    InitTestRepo();

    // 1. Create a blocked .env file
    WriteFile(repo.path() / ".env", "DATABASE_URL=postgres://user:pass@localhost/db\n");

    CaptureStdout cap;
    cmd_add(".env", /*forceAdd=*/false);

    // Staging index must remain empty because .env was blocked
    Tree index = loadIndex();
    EXPECT_EQ(index.size(), 0U);
    EXPECT_OUTPUT_CONTAINS(cap, "BLOCKED");
}

TEST(CliIntegrationTest, AddAllowsCleanFilesAndForceOverride) {
    TempRepo repo("cli_add_clean");
    InitTestRepo();

    // 1. Clean file
    WriteFile(repo.path() / "app.cpp", "int main() { return 0; }\n");
    {
        CaptureStdout cap;
        cmd_add("app.cpp", /*forceAdd=*/false);
    }
    Tree index = loadIndex();
    EXPECT_EQ(index.size(), 1U);
    EXPECT_TRUE(index.find("app.cpp") != index.end());

    // 2. Force add of .env with warning
    WriteFile(repo.path() / ".env", "API_KEY=test\n");
    {
        CaptureStdout cap;
        cmd_add(".env", /*forceAdd=*/true);
    }
    index = loadIndex();
    EXPECT_EQ(index.size(), 2U);
    EXPECT_TRUE(index.find(".env") != index.end());
}

TEST(CliIntegrationTest, CommitCreatesObjectAndAdvancesHead) {
    TempRepo repo("cli_commit");
    InitTestRepo();

    WriteFile(repo.path() / "file.txt", "First version content\n");
    cmd_add("file.txt");

    CaptureStdout cap;
    cmd_commit("Initial commit");

    // HEAD must point to a non-empty commit identifier
    std::string headId = headCommitId();
    EXPECT_FALSE(headId.empty());
    EXPECT_NE(headId.find("commit_"), std::string::npos);

    // Commit metadata must exist and match
    auto meta = loadCommitMeta(headId);
    ASSERT_TRUE(meta.has_value());
    EXPECT_EQ(meta->message, "Initial commit");
    EXPECT_EQ(meta->branch, "main");

    // Index must be empty after commit
    Tree indexAfter = loadIndex();
    EXPECT_EQ(indexAfter.size(), 0U);
}

TEST(CliIntegrationTest, BranchCreationAndSwitching) {
    TempRepo repo("cli_branch");
    InitTestRepo();

    WriteFile(repo.path() / "README.md", "# My Project\n");
    cmd_add("README.md");
    cmd_commit("Commit on main");

    EXPECT_EQ(currentBranch(), "main");

    // Create branch "feature-auth"
    {
        CaptureStdout cap;
        cmd_branch("create", "feature-auth");
        EXPECT_OUTPUT_CONTAINS(cap, "feature-auth");
    }
    EXPECT_FILE_EXISTS(".miniVCS/refs/heads/feature-auth");

    // Switch to "feature-auth"
    {
        CaptureStdout cap;
        cmd_branch("checkout", "feature-auth");
    }
    EXPECT_EQ(currentBranch(), "feature-auth");
}

TEST(CliIntegrationTest, ResetClearsStagingIndex) {
    TempRepo repo("cli_reset");
    InitTestRepo();

    WriteFile(repo.path() / "staged.txt", "Some content\n");
    cmd_add("staged.txt");

    EXPECT_EQ(loadIndex().size(), 1U);

    CaptureStdout cap;
    cmd_reset();

    EXPECT_EQ(loadIndex().size(), 0U);
    EXPECT_OUTPUT_CONTAINS(cap, "Staging area cleared");
}

TEST(CliIntegrationTest, VersionRecoveryViaCheckout) {
    TempRepo repo("cli_recovery");
    InitTestRepo();

    // Commit 1: v1
    WriteFile(repo.path() / "doc.txt", "Version 1.0\n");
    cmd_add("doc.txt");
    cmd_commit("Release 1.0");
    std::string commit1 = headCommitId();

    // Commit 2: v2
    WriteFile(repo.path() / "doc.txt", "Version 2.0 with breaking changes\n");
    cmd_add("doc.txt");
    cmd_commit("Release 2.0");
    std::string commit2 = headCommitId();

    EXPECT_NE(commit1, commit2);
    EXPECT_FILE_CONTAINS(repo.path() / "doc.txt", "Version 2.0");

    // Checkout commit 1 to recover v1
    {
        CaptureStdout cap;
        cmd_checkout(commit1);
        EXPECT_OUTPUT_CONTAINS(cap, "Checked out");
    }

    // Verify file content was restored to v1
    EXPECT_FILE_CONTAINS(repo.path() / "doc.txt", "Version 1.0");
}

TEST(CliIntegrationTest, CommitSavesMessageWithDashMFlag) {
    TempRepo repo("cli_commit_m_flag");
    InitTestRepo();

    WriteFile(repo.path() / "feature.cpp", "// new feature code\n");
    cmd_add("feature.cpp");

    CaptureStdout cap;
    dispatchCommit({"-m", "Add user authentication module"});

    std::string headId = headCommitId();
    ASSERT_FALSE(headId.empty());
    auto meta = loadCommitMeta(headId);
    ASSERT_TRUE(meta.has_value());
    EXPECT_EQ(meta->message, "Add user authentication module");
    EXPECT_OUTPUT_CONTAINS(cap, "Add user authentication module");
}

TEST(CliIntegrationTest, CommitSavesMessageWithUnquotedPositionalWords) {
    TempRepo repo("cli_commit_unquoted");
    InitTestRepo();

    WriteFile(repo.path() / "fix.txt", "bug fix\n");
    cmd_add("fix.txt");

    CaptureStdout cap;
    dispatchCommit({"Fix", "critical", "memory", "leak", "in", "parser"});

    std::string headId = headCommitId();
    ASSERT_FALSE(headId.empty());
    auto meta = loadCommitMeta(headId);
    ASSERT_TRUE(meta.has_value());
    EXPECT_EQ(meta->message, "Fix critical memory leak in parser");
}

TEST(CliIntegrationTest, CommitSavesMultiLineWithMultipleMFlags) {
    TempRepo repo("cli_commit_multi_m");
    InitTestRepo();

    WriteFile(repo.path() / "module.cpp", "int x = 42;\n");
    cmd_add("module.cpp");

    CaptureStdout cap;
    dispatchCommit({"-m", "Release v1.2", "-m", "- Added S3 storage backend", "-m", "- Fixed commit message parsing"});

    std::string headId = headCommitId();
    ASSERT_FALSE(headId.empty());
    auto meta = loadCommitMeta(headId);
    ASSERT_TRUE(meta.has_value());
    EXPECT_EQ(meta->message, "Release v1.2\n- Added S3 storage backend\n- Fixed commit message parsing");
}

TEST(CliIntegrationTest, CommitSavesMessageWithEqualsSyntax) {
    TempRepo repo("cli_commit_equals");
    InitTestRepo();

    WriteFile(repo.path() / "config.ini", "key=value\n");
    cmd_add("config.ini");

    CaptureStdout cap;
    dispatchCommit({"-m=chore: update configuration"});

    std::string headId = headCommitId();
    ASSERT_FALSE(headId.empty());
    auto meta = loadCommitMeta(headId);
    ASSERT_TRUE(meta.has_value());
    EXPECT_EQ(meta->message, "chore: update configuration");
}

TEST(CliIntegrationTest, CommitFallbackDefaultWhenPromptEmpty) {
    TempRepo repo("cli_commit_empty");
    InitTestRepo();

    WriteFile(repo.path() / "auto.txt", "data\n");
    cmd_add("auto.txt");

    FakeStdin input("\n"); // Hit Enter on prompt
    CaptureStdout cap;
    cmd_commit("");

    std::string headId = headCommitId();
    ASSERT_FALSE(headId.empty());
    auto meta = loadCommitMeta(headId);
    ASSERT_TRUE(meta.has_value());
    EXPECT_EQ(meta->message, "Update files (1 staged)");
}

TEST(CliIntegrationTest, StatusDisplaysStagedNewAndModified) {
    TempRepo repo("cli_status");
    InitTestRepo();

    // 1. Initial file committed
    WriteFile(repo.path() / "tracked.txt", "v1 content\n");
    cmd_add("tracked.txt");
    cmd_commit("Commit v1");

    // 2. Modify tracked.txt and add new file
    WriteFile(repo.path() / "tracked.txt", "v2 modified content\n");
    WriteFile(repo.path() / "brand_new.txt", "fresh\n");
    cmd_add("tracked.txt");
    cmd_add("brand_new.txt");

    CaptureStdout cap;
    cmd_status();
    EXPECT_OUTPUT_CONTAINS(cap, "Staged changes:");
    EXPECT_OUTPUT_CONTAINS(cap, "brand_new.txt");
    EXPECT_OUTPUT_CONTAINS(cap, "tracked.txt");
}

TEST(CliIntegrationTest, LogDisplaysCommitHistoryAndAuthor) {
    TempRepo repo("cli_log");
    InitTestRepo();

    WriteFile(repo.path() / "file1.txt", "1\n");
    cmd_add("file1.txt");
    dispatchCommit({"-m", "First feature implemented"});

    WriteFile(repo.path() / "file2.txt", "2\n");
    cmd_add("file2.txt");
    dispatchCommit({"-m", "Second bug fix applied"});

    CaptureStdout cap;
    cmd_log();
    EXPECT_OUTPUT_CONTAINS(cap, "First feature implemented");
    EXPECT_OUTPUT_CONTAINS(cap, "Second bug fix applied");
    EXPECT_OUTPUT_CONTAINS(cap, "commit_");
}

TEST(CliIntegrationTest, DiffDisplaysLineDifferences) {
    TempRepo repo("cli_diff");
    InitTestRepo();

    WriteFile(repo.path() / "code.txt", "Line A\nLine B\n");
    cmd_add("code.txt");
    cmd_commit("Initial lines");

    std::string c1 = headCommitId();

    WriteFile(repo.path() / "code.txt", "Line A\nLine B modified\nLine C\n");
    cmd_add("code.txt");
    cmd_commit("Modified lines");

    std::string c2 = headCommitId();

    CaptureStdout cap;
    cmd_diff(c1, c2);
    EXPECT_OUTPUT_CONTAINS(cap, "diff ");
    EXPECT_OUTPUT_CONTAINS(cap, "code.txt");
}

TEST(CliIntegrationTest, ConfigGetSetListOperations) {
    TempRepo repo("cli_config");
    InitTestRepo();

    // Set config value
    {
        CaptureStdout cap;
        cmd_config("set", {"project_env", "staging"});
        EXPECT_OUTPUT_CONTAINS(cap, "Set project_env=staging");
    }

    // Get config value
    {
        CaptureStdout cap;
        cmd_config("get", {"project_env"});
        EXPECT_OUTPUT_CONTAINS(cap, "project_env=staging");
    }

    // List all config values
    {
        CaptureStdout cap;
        cmd_config("list", {});
        EXPECT_OUTPUT_CONTAINS(cap, "project_env=staging");
        EXPECT_OUTPUT_CONTAINS(cap, "backend=");
    }
}

TEST(CliIntegrationTest, RemoteShowAndSetOperations) {
    TempRepo repo("cli_remote");
    InitTestRepo();

    // Show remote settings
    {
        CaptureStdout cap;
        cmd_remote("show", {});
        EXPECT_OUTPUT_CONTAINS(cap, "Backend:");
        EXPECT_OUTPUT_CONTAINS(cap, "Bucket:");
    }

    // Update remote bucket
    {
        CaptureStdout cap;
        cmd_remote("set", {"bucket", "my-custom-backup-store"});
        EXPECT_OUTPUT_CONTAINS(cap, "Set bucket=my-custom-backup-store");
    }

    EXPECT_EQ(cfgGet("bucket"), "my-custom-backup-store");
}

TEST(CliIntegrationTest, BranchListAndDelete) {
    TempRepo repo("cli_branch_mgmt");
    InitTestRepo();

    WriteFile(repo.path() / "f.txt", "base\n");
    cmd_add("f.txt");
    cmd_commit("Base commit");

    // Create a new branch
    cmd_branch("create", "release-v2");
    EXPECT_FILE_EXISTS(".miniVCS/refs/heads/release-v2");

    // List branches
    {
        CaptureStdout cap;
        cmd_branch("list");
        EXPECT_OUTPUT_CONTAINS(cap, "main");
        EXPECT_OUTPUT_CONTAINS(cap, "release-v2");
    }

    // Delete branch
    {
        CaptureStdout cap;
        cmd_branch("delete", "release-v2");
        EXPECT_OUTPUT_CONTAINS(cap, "Branch 'release-v2' deleted");
    }
    EXPECT_FILE_NOT_EXISTS(".miniVCS/refs/heads/release-v2");
}

