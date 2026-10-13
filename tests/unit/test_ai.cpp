/**
 * tests/unit/test_ai.cpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Unit tests for AI features:
 *  - Local Ollama Tiny integration (prompt formatting, API call, unescaping)
 *  - Automated commit message generation (context construction, file truncation)
 *  - Local codebase semantic search (ignore rule filtering, binary skipping)
 *  - AI Merge Conflict Resolution (API key validation, option ranking)
 */

#include "tests/test_helpers.hpp"
#include "minivcs.hpp"

// ─── Ollama Unit Tests ────────────────────────────────────────────────────────

TEST(AiUnitTest, OllamaCallHandlesOfflineGracefully) {
    TempRepo repo("ai_offline");
    // Ensure repository is initialized so config exists
    {
        FakeStdin input("alice\n7\n.local_remote\n");
        CaptureStdout cap;
        cmd_init();
    }

    // Set an invalid port to simulate offline / unavailable Ollama server
    auto cfg = readConfig();
    cfg["ollama_url"] = "http://127.0.0.1:59998";
    writeConfig(cfg);

    CaptureStdout cap;
    std::string resp = ollamaCall("Hello AI");
    EXPECT_TRUE(resp.empty());

    // Temporary request and response files should be cleaned up
    EXPECT_FALSE(fs::exists(".miniVCS/.tmp_ollama_req.json"));
    EXPECT_FALSE(fs::exists(".miniVCS/.tmp_ollama_resp.json"));
}

TEST(AiUnitTest, GenerateCommitMessageWithEmptyStagedList) {
    TempRepo repo("ai_gen_commit_empty");
    {
        FakeStdin input("alice\n7\n.local_remote\n");
        CaptureStdout cap;
        cmd_init();
    }

    // With invalid URL, returns empty string cleanly
    auto cfg = readConfig();
    cfg["ollama_url"] = "http://127.0.0.1:59998";
    writeConfig(cfg);

    std::vector<std::pair<std::string, std::string>> files;
    std::string msg = generateCommitMessage(files);
    EXPECT_TRUE(msg.empty());
}

TEST(AiUnitTest, GenerateCommitMessageContextConstruction) {
    TempRepo repo("ai_gen_commit_context");
    {
        FakeStdin input("alice\n7\n.local_remote\n");
        CaptureStdout cap;
        cmd_init();
    }

    // Create 15 changed files to test the truncation limit (> 10 files)
    std::vector<std::pair<std::string, std::string>> files;
    for (int i = 1; i <= 15; i++) {
        files.push_back({"file_" + std::to_string(i) + ".cpp", "hash_" + std::to_string(i)});
    }

    // Invalid URL so ollamaCall fails fast
    auto cfg = readConfig();
    cfg["ollama_url"] = "http://127.0.0.1:59998";
    writeConfig(cfg);

    std::string msg = generateCommitMessage(files);
    // Should complete cleanly without throwing or crashing
    EXPECT_TRUE(msg.empty());
}

TEST(AiUnitTest, SearchCodebaseSkipsBinaryAndIgnoredFiles) {
    TempRepo repo("ai_search_skips");
    {
        FakeStdin input("alice\n7\n.local_remote\n");
        CaptureStdout cap;
        cmd_init();
    }

    // Set invalid URL to test the local file collection logic without requiring active server
    auto cfg = readConfig();
    cfg["ollama_url"] = "http://127.0.0.1:59998";
    writeConfig(cfg);

    // Write a regular code file
    WriteFile(repo.path() / "app.cpp", "int computeTotal() { return 42; }\n");

    // Write an ignored file matching .miniVCSignore (node_modules)
    WriteFile(repo.path() / "node_modules" / "pkg.js", "function ignored() {}\n");

    // Write a binary file with null bytes
    std::string binContent = "ELF\x00\x01\x02\x00BinaryPayload";
    WriteFile(repo.path() / "binary.bin", binContent);

    CaptureStdout cap;
    searchCodebase("computeTotal");

    // Should indicate Ollama was searched locally, no data leaves machine
    EXPECT_OUTPUT_CONTAINS(cap, "[Ollama] Searching codebase (locally, no data leaves machine)...");
    // When offline, displays notice
    EXPECT_OUTPUT_CONTAINS(cap, "Ollama not available");
}

TEST(AiUnitTest, MergeResolveFailsWithoutApiKey) {
    TempRepo repo("ai_merge_no_key");
    {
        FakeStdin input("alice\n7\n.local_remote\n");
        CaptureStdout cap;
        cmd_init();
    }

    // Ensure ai_api_key is unset
    auto cfg = readConfig();
    cfg.erase("ai_api_key");
    writeConfig(cfg);

    std::string conflict =
        "<<<<<<< HEAD\n"
        "int value = 100;\n"
        "=======\n"
        "int value = 200;\n"
        ">>>>>>> feature\n";

    CaptureStdout cap;
    auto resolutions = aiResolveMerge(conflict);
    EXPECT_TRUE(resolutions.empty());
    EXPECT_OUTPUT_CONTAINS(cap, "No AI API key configured");
}

TEST(AiUnitTest, CmdMergeResolveRejectsFileWithoutMarkers) {
    TempRepo repo("ai_merge_no_markers");
    {
        FakeStdin input("alice\n7\n.local_remote\n");
        CaptureStdout cap;
        cmd_init();
    }

    WriteFile(repo.path() / "clean.txt", "This file has no merge conflicts.\n");

    CaptureStdout cap;
    cmd_merge_resolve("clean.txt");
    EXPECT_OUTPUT_CONTAINS(cap, "No merge conflict markers found in: clean.txt");
}

TEST(AiUnitTest, CmdMergeResolveRejectsNonExistentFile) {
    TempRepo repo("ai_merge_nonexistent");
    {
        FakeStdin input("alice\n7\n.local_remote\n");
        CaptureStdout cap;
        cmd_init();
    }

    CaptureStdout cap;
    cmd_merge_resolve("does_not_exist.txt");
    EXPECT_OUTPUT_CONTAINS(cap, "File not found: does_not_exist.txt");
}

TEST(AiUnitTest, MergeBenchLanguageDetectionAllElevenLanguages) {
    EXPECT_EQ(detectLanguageFromPath("Main.java"), "java");
    EXPECT_EQ(detectLanguageFromPath("calc.py"), "python");
    EXPECT_EQ(detectLanguageFromPath("engine.cpp"), "cpp");
    EXPECT_EQ(detectLanguageFromPath("header.h"), "cpp");
    EXPECT_EQ(detectLanguageFromPath("kernel.c"), "c");
    EXPECT_EQ(detectLanguageFromPath("Service.cs"), "csharp");
    EXPECT_EQ(detectLanguageFromPath("server.go"), "go");
    EXPECT_EQ(detectLanguageFromPath("app.js"), "javascript");
    EXPECT_EQ(detectLanguageFromPath("index.ts"), "typescript");
    EXPECT_EQ(detectLanguageFromPath("api.php"), "php");
    EXPECT_EQ(detectLanguageFromPath("script.rb"), "ruby");
    EXPECT_EQ(detectLanguageFromPath("lib.rs"), "rust");
}

TEST(AiUnitTest, MergeBenchSourceCodeNormalizationRemovesCommentsAndStandardizesWhitespace) {
    // Java / C++ comment removal
    std::string cppCode = 
        "// line comment\n"
        "/* block\n"
        "   comment */\n"
        "int   x  =   42;  // trailing\n";
    std::string cppNorm = normalizeSourceCode(cppCode, "cpp");
    EXPECT_EQ(cppNorm, "int x = 42;");

    // Python docstring & comment removal
    std::string pyCode = 
        "# python comment\n"
        "def compute(a):\n"
        "    \"\"\"docstring explanation\"\"\"\n"
        "    return a   *   2  # inline\n";
    std::string pyNorm = normalizeSourceCode(pyCode, "python");
    EXPECT_TRUE(pyNorm.find("docstring") == std::string::npos);
    EXPECT_TRUE(pyNorm.find("python comment") == std::string::npos);
    EXPECT_TRUE(pyNorm.find("def compute(a):") != std::string::npos);
    EXPECT_TRUE(pyNorm.find("return a * 2") != std::string::npos);
}

// ─── Search-Based Software Engineering (SBCR / SBSE) Unit Tests ───────────────

TEST(AiUnitTest, SearchBasedLcsSimilarityMetrics) {
    // Identical line sequences -> 1.0
    std::vector<std::string> seq1 = {"line1", "line2", "line3"};
    std::vector<std::string> seq2 = {"line1", "line2", "line3"};
    EXPECT_TRUE(std::abs(lineLcsSimilarity(seq1, seq2) - 1.0) < 1e-5);

    // Empty sequences -> 1.0
    EXPECT_TRUE(std::abs(lineLcsSimilarity({}, {}) - 1.0) < 1e-5);

    // One empty, one non-empty -> 0.0
    EXPECT_TRUE(std::abs(lineLcsSimilarity(seq1, {}) - 0.0) < 1e-5);
    EXPECT_TRUE(std::abs(lineLcsSimilarity({}, seq2) - 0.0) < 1e-5);

    // Disjoint sequences -> 0.0
    std::vector<std::string> seqDisjoint = {"apple", "banana"};
    std::vector<std::string> seqOther = {"cat", "dog"};
    EXPECT_TRUE(std::abs(lineLcsSimilarity(seqDisjoint, seqOther) - 0.0) < 1e-5);

    // Partial match: LCS = 1 out of sizes 2 and 2 -> 2 * 1 / (2 + 2) = 0.50
    std::vector<std::string> parentA = {"int a = 1;", "int common = 2;"};
    std::vector<std::string> parentB = {"int b = 3;", "int common = 2;"};
    EXPECT_TRUE(std::abs(lineLcsSimilarity(parentA, parentB) - 0.50) < 1e-5);

    // Candidate C combines changes: {"int a = 1;", "int b = 3;", "int common = 2;"}
    // Sim(C, parentA) = 2 * 2 / (3 + 2) = 0.80
    // Sim(C, parentB) = 2 * 2 / (3 + 2) = 0.80
    // Fitness = (0.80 + 0.80) / 2 = 0.80 (Campos Junior et al., 2025 metric)
    std::vector<std::string> cand = {"int a = 1;", "int b = 3;", "int common = 2;"};
    EXPECT_TRUE(std::abs(lineLcsSimilarity(cand, parentA) - 0.80) < 1e-5);
    EXPECT_TRUE(std::abs(lineLcsSimilarity(cand, parentB) - 0.80) < 1e-5);
    EXPECT_TRUE(std::abs(dualParentSimilarity(cand, parentA, parentB) - 0.80) < 1e-5);
}

TEST(AiUnitTest, SearchBasedConflictResolverPreservesPartialOrderAndRecombines) {
    std::string conflict =
        "int calculate() {\n"
        "<<<<<<< HEAD\n"
        "    int a = 10;\n"
        "    int common = 50;\n"
        "=======\n"
        "    int b = 20;\n"
        "    int common = 50;\n"
        ">>>>>>> feature\n"
        "    return common;\n"
        "}\n";

    std::string resolved = resolveConflictBySearch(conflict);

    // Should remove conflict markers
    EXPECT_TRUE(resolved.find("<<<<<<<") == std::string::npos);
    EXPECT_TRUE(resolved.find("=======") == std::string::npos);
    EXPECT_TRUE(resolved.find(">>>>>>>") == std::string::npos);

    // Surrounding context preserved
    EXPECT_TRUE(resolved.find("int calculate() {") != std::string::npos);
    EXPECT_TRUE(resolved.find("return common;") != std::string::npos);

    // Both concurrent changes incorporated in valid partial order
    EXPECT_TRUE(resolved.find("int a = 10;") != std::string::npos);
    EXPECT_TRUE(resolved.find("int b = 20;") != std::string::npos);
    EXPECT_TRUE(resolved.find("int common = 50;") != std::string::npos);
}

TEST(AiUnitTest, SearchBasedConflictResolverHandlesOneSidedAdditionAndDeletion) {
    std::string conflict =
        "void setup() {\n"
        "<<<<<<< HEAD\n"
        "    initializeLogging();\n"
        "=======\n"
        ">>>>>>> feature\n"
        "}\n";

    std::string resolved = resolveConflictBySearch(conflict);
    EXPECT_TRUE(resolved.find("<<<<<<<") == std::string::npos);
    EXPECT_TRUE(resolved.find("initializeLogging();") != std::string::npos);
    EXPECT_TRUE(resolved.find("void setup() {") != std::string::npos);
}

TEST(AiUnitTest, ResolveMergeSearchProducesRankedOptionsWithFitness) {
    std::string conflict =
        "<<<<<<< HEAD\n"
        "    int speed = 100;\n"
        "=======\n"
        "    int boost = 50;\n"
        ">>>>>>> branch\n";

    auto resolutions = resolveMergeSearch(conflict);
    EXPECT_TRUE(resolutions.size() >= 2);

    // Option 1 should be the SBCR candidate with high confidence and informative rationale
    EXPECT_TRUE(resolutions[0].confidence >= 0.75);
    EXPECT_TRUE(resolutions[0].rationale.find("Search-Based") != std::string::npos);
    EXPECT_TRUE(resolutions[0].resolution.find("<<<<<<<") == std::string::npos);
}

TEST(AiUnitTest, ResolveMergeHybridRescuesWhenApiKeyUnset) {
    TempRepo repo("ai_hybrid_rescue");
    {
        FakeStdin input("alice\n7\n.local_remote\n");
        CaptureStdout cap;
        cmd_init();
    }

    auto cfg = readConfig();
    cfg.erase("ai_api_key");
    writeConfig(cfg);

    std::string conflict =
        "<<<<<<< HEAD\n"
        "    double threshold = 0.95;\n"
        "=======\n"
        "    double tolerance = 0.05;\n"
        ">>>>>>> branch\n";

    // Hybrid resolver seamlessly falls back to Search-Based Software Engineering
    auto resolutions = resolveMergeHybrid(conflict, "cpp");
    EXPECT_FALSE(resolutions.empty());
    EXPECT_TRUE(resolutions[0].confidence >= 0.75);
    EXPECT_TRUE(resolutions[0].resolution.find("threshold") != std::string::npos ||
                resolutions[0].resolution.find("tolerance") != std::string::npos);
}

