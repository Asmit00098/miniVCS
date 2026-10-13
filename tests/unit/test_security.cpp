/**
 * tests/unit/test_security.cpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Unit tests for Security Guardrails:
 *  - Filename blocklist (exact match, wildcards, case-insensitivity)
 *  - Secret scanner regex patterns (AWS, GitHub, Slack, DB URI, Private Keys)
 *  - Clean file pass-through
 *  - Binary file exclusion
 */

#include "tests/test_helpers.hpp"
#include "minivcs.hpp"

// ─── Filename Blocklist Tests ─────────────────────────────────────────────────

TEST(SecurityTest, BlockedDotEnvVariations) {
    EXPECT_TRUE(matchesBlockedFilename(".env"));
    EXPECT_TRUE(matchesBlockedFilename(".env.local"));
    EXPECT_TRUE(matchesBlockedFilename(".env.development"));
    EXPECT_TRUE(matchesBlockedFilename(".env.production"));
    EXPECT_TRUE(matchesBlockedFilename(".env.test"));
    EXPECT_TRUE(matchesBlockedFilename(".env.staging"));
}

TEST(SecurityTest, BlockedKeyAndCertExtensions) {
    EXPECT_TRUE(matchesBlockedFilename("server.pem"));
    EXPECT_TRUE(matchesBlockedFilename("domain.key"));
    EXPECT_TRUE(matchesBlockedFilename("certificate.p12"));
    EXPECT_TRUE(matchesBlockedFilename("identity.pfx"));
}

TEST(SecurityTest, BlockedSshAndCredentialFiles) {
    EXPECT_TRUE(matchesBlockedFilename("id_rsa"));
    EXPECT_TRUE(matchesBlockedFilename("id_dsa"));
    EXPECT_TRUE(matchesBlockedFilename("id_ecdsa"));
    EXPECT_TRUE(matchesBlockedFilename("id_ed25519"));
    EXPECT_TRUE(matchesBlockedFilename(".netrc"));
    EXPECT_TRUE(matchesBlockedFilename("credentials"));
    EXPECT_TRUE(matchesBlockedFilename("secrets.yml"));
    EXPECT_TRUE(matchesBlockedFilename("secrets.yaml"));
    EXPECT_TRUE(matchesBlockedFilename("secrets.json"));
}

TEST(SecurityTest, BlocklistCaseInsensitive) {
    EXPECT_TRUE(matchesBlockedFilename(".ENV"));
    EXPECT_TRUE(matchesBlockedFilename(".Env.Local"));
    EXPECT_TRUE(matchesBlockedFilename("SERVER.PEM"));
    EXPECT_TRUE(matchesBlockedFilename("ID_RSA"));
    EXPECT_TRUE(matchesBlockedFilename("SECRETS.JSON"));
}

TEST(SecurityTest, AllowedSafeFiles) {
    EXPECT_FALSE(matchesBlockedFilename("main.cpp"));
    EXPECT_FALSE(matchesBlockedFilename("Readme.md"));
    EXPECT_FALSE(matchesBlockedFilename("package.json"));
    EXPECT_FALSE(matchesBlockedFilename("index.html"));
    EXPECT_FALSE(matchesBlockedFilename("config.example.json"));
    EXPECT_FALSE(matchesBlockedFilename("environment.ts"));
}

// ─── Secret Scanner Tests ────────────────────────────────────────────────────

TEST(SecurityTest, DetectsAwsAccessKey) {
    TempRepo repo("sec_aws_key");
    fs::path f = repo.path() / "config.js";
    WriteFile(f, "const AWS_KEY = 'AKIAIOSFODNN7EXAMPLE';\n");

    auto matches = scanFileForSecrets(f);
    ASSERT_EQ(matches.size(), 1U);
    EXPECT_EQ(matches[0].rule, "AWS Access Key");
    EXPECT_EQ(matches[0].lineNo, 1);
}

TEST(SecurityTest, DetectsPrivateKeyHeader) {
    TempRepo repo("sec_priv_key");
    fs::path f = repo.path() / "token.txt";
    WriteFile(f, "# Header line\n-----BEGIN RSA PRIVATE KEY-----\nMIIEowIBAAKCAQEA...\n");

    auto matches = scanFileForSecrets(f);
    ASSERT_EQ(matches.size(), 1U);
    EXPECT_EQ(matches[0].rule, "Private Key Header");
    EXPECT_EQ(matches[0].lineNo, 2);
}

TEST(SecurityTest, DetectsGithubToken) {
    TempRepo repo("sec_github");
    fs::path f = repo.path() / "ci.sh";
    WriteFile(f, "export GITHUB_TOKEN=\"ghp_0123456789abcdefghijklmnopqrstuvwxyz012\"\n");

    auto matches = scanFileForSecrets(f);
    EXPECT_TRUE(matches.size() >= 1U);
    bool found = false;
    for(const auto& m : matches) if(m.rule == "GitHub Token") found = true;
    EXPECT_TRUE(found);
}

TEST(SecurityTest, DetectsSlackToken) {
    TempRepo repo("sec_slack");
    fs::path f = repo.path() / "bot.py";
    WriteFile(f, "SLACK_BOT_TOKEN = 'xoxb-1234567890-abcdefghij'\n");

    auto matches = scanFileForSecrets(f);
    EXPECT_TRUE(matches.size() >= 1U);
    bool found = false;
    for(const auto& m : matches) if(m.rule == "Slack Token") found = true;
    EXPECT_TRUE(found);
}

TEST(SecurityTest, DetectsDatabaseUrl) {
    TempRepo repo("sec_db_url");
    fs::path f = repo.path() / "db.py";
    WriteFile(f, "DB_URI = 'postgresql://admin:supersecretpassword@db.prod.internal:5432/core'\n");

    auto matches = scanFileForSecrets(f);
    ASSERT_EQ(matches.size(), 1U);
    EXPECT_EQ(matches[0].rule, "Database URL");
}

TEST(SecurityTest, CleanFileHasZeroFindings) {
    TempRepo repo("sec_clean");
    fs::path f = repo.path() / "app.cpp";
    WriteFile(f, "#include <iostream>\nint main() { std::cout << \"Hello World\\n\"; return 0; }\n");

    auto matches = scanFileForSecrets(f);
    EXPECT_EQ(matches.size(), 0U);
}

TEST(SecurityTest, BinaryFileIsSkipped) {
    TempRepo repo("sec_binary");
    fs::path f = repo.path() / "binary.dat";
    // Embed a null byte at offset 4 followed by what would look like an AWS key
    std::string binData = "BIN\0AKIAIOSFODNN7EXAMPLE\0MORE";
    std::ofstream out(f, std::ios::binary);
    out.write(binData.data(), binData.size());
    out.close();

    auto matches = scanFileForSecrets(f);
    EXPECT_EQ(matches.size(), 0U);
}
