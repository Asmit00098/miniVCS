/**
 * tests/integration/test_ai_live.cpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Live integration tests connecting to a running local Ollama instance:
 *  - Live query inference (ollamaCall)
 *  - Automatic commit message generation (generateCommitMessage)
 *  - Local codebase semantic search (searchCodebase)
 *  - Commit workflow with --ai flag
 */

#include "tests/test_helpers.hpp"
#include "minivcs.hpp"

// Helper to check if Ollama server and tinyllama are reachable
inline bool IsOllamaTinyReady() {
    // Probe localhost:11434 directly
    int rc = system("curl -s -f http://localhost:11434/api/tags > nul 2>&1");
    if (rc != 0) return false;

    // Check if tinyllama is present in tags
    std::string tmp = ".miniVCS/.probe_tags.json";
    fs::create_directories(".miniVCS");
    std::string cmd = "curl -s http://localhost:11434/api/tags -o \"" + tmp + "\" 2>&1";
    system(cmd.c_str());
    std::string tags = ReadFile(tmp);
    fs::remove(tmp);
    return tags.find("tinyllama") != std::string::npos;
}

TEST(AiLiveIntegrationTest, LiveOllamaCallProducesResponse) {
    if (!IsOllamaTinyReady()) {
        std::cout << "[SKIP] Ollama or tinyllama model not ready yet.\n";
        return;
    }

    TempRepo repo("ai_live_call");
    FakeStdin input("tester\n7\n.local_remote\n");
    CaptureStdout cap;
    cmd_init();

    std::string response = ollamaCall("Reply with the single word: OK", "tinyllama");
    EXPECT_FALSE(response.empty());
}

TEST(AiLiveIntegrationTest, LiveGenerateCommitMessageFromStagedDiff) {
    if (!IsOllamaTinyReady()) {
        std::cout << "[SKIP] Ollama or tinyllama model not ready yet.\n";
        return;
    }

    TempRepo repo("ai_live_commit_gen");
    FakeStdin input("tester\n7\n.local_remote\n");
    CaptureStdout cap;
    cmd_init();

    std::vector<std::pair<std::string, std::string>> changedFiles = {
        {"src/auth/jwt_validator.cpp", "hash_abc123"},
        {"src/auth/permissions.hpp", "hash_def456"}
    };

    std::string msg = generateCommitMessage(changedFiles);
    EXPECT_FALSE(msg.empty());
}

TEST(AiLiveIntegrationTest, LiveSearchCodebaseDiscoversFunction) {
    if (!IsOllamaTinyReady()) {
        std::cout << "[SKIP] Ollama or tinyllama model not ready yet.\n";
        return;
    }

    TempRepo repo("ai_live_search");
    {
        FakeStdin input("tester\n7\n.local_remote\n");
        CaptureStdout cap;
        cmd_init();
    }

    WriteFile(repo.path() / "encryption.cpp",
              "// AES-256 GCM encryption routine\n"
              "void encryptDatabasePayload(const std::string& key) {\n"
              "    // secure crypto logic\n"
              "}\n");

    CaptureStdout cap;
    searchCodebase("encryptDatabasePayload");
    EXPECT_OUTPUT_CONTAINS(cap, "[Ollama] Searching codebase");
    EXPECT_OUTPUT_CONTAINS(cap, "[Search Results]");
}

TEST(AiLiveIntegrationTest, LiveCommitWithAiFlagSavesGeneratedMessage) {
    if (!IsOllamaTinyReady()) {
        std::cout << "[SKIP] Ollama or tinyllama model not ready yet.\n";
        return;
    }

    TempRepo repo("ai_live_commit_workflow");
    {
        FakeStdin input("tester\n7\n.local_remote\n");
        CaptureStdout cap;
        cmd_init();
    }

    WriteFile(repo.path() / "service.cpp", "int computeMetric() { return 100; }\n");
    cmd_add("service.cpp");

    // In automated context, simulate accepting generated message with "Y\n" (and fallback if empty)
    FakeStdin input("Y\nAutocommit message\n");
    CaptureStdout cap;
    dispatchCommit({"--ai"});

    std::string headId = headCommitId();
    ASSERT_FALSE(headId.empty());
    auto cm = loadCommitMeta(headId);
    ASSERT_TRUE(cm.has_value());
    EXPECT_FALSE(cm->message.empty());
    EXPECT_OUTPUT_CONTAINS(cap, "Committed: " + headId);
}
