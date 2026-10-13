/**
 * tests/unit/test_config.cpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Unit tests for repository configuration & ignore file parser:
 *  - Config read/write and default fallbacks
 *  - Multi-cloud storage backend URL generation (S3, GCS, Azure, B2, MinIO, SFTP, Local)
 *  - .miniVCSignore rule parsing (comments, empty lines, wildcards, directories)
 */

#include "tests/test_helpers.hpp"
#include "minivcs.hpp"

// ─── Config Tests ─────────────────────────────────────────────────────────────

TEST(ConfigTest, ReadWriteConfig) {
    TempRepo repo("cfg_read_write");
    fs::create_directories(".miniVCS");

    std::map<std::string,std::string> cfg;
    cfg["username"] = "alice";
    cfg["backend"] = "s3";
    cfg["bucket"] = "alice-vault";
    writeConfig(cfg);

    auto loaded = readConfig();
    EXPECT_EQ(loaded["username"], "alice");
    EXPECT_EQ(loaded["backend"], "s3");
    EXPECT_EQ(loaded["bucket"], "alice-vault");
}

TEST(ConfigTest, CfgGetWithDefaultFallback) {
    TempRepo repo("cfg_defaults");
    fs::create_directories(".miniVCS");

    std::map<std::string,std::string> cfg;
    cfg["active_key"] = "present";
    writeConfig(cfg);

    EXPECT_EQ(cfgGet("active_key", "default_val"), "present");
    EXPECT_EQ(cfgGet("missing_key", "fallback_val"), "fallback_val");
}

TEST(ConfigTest, StorageBackendRemotePrefixes) {
    StorageBackend b;
    b.bucket = "my-bucket";
    b.username = "team_alpha";

    b.type = "s3";
    EXPECT_EQ(remotePrefix(b), "s3://my-bucket/team_alpha");

    b.type = "gcs";
    EXPECT_EQ(remotePrefix(b), "gs://my-bucket/team_alpha");

    b.type = "azure";
    EXPECT_EQ(remotePrefix(b), "azure://my-bucket/team_alpha");

    b.type = "b2";
    EXPECT_EQ(remotePrefix(b), "b2://my-bucket/team_alpha");

    b.type = "minio";
    EXPECT_EQ(remotePrefix(b), "s3://my-bucket/team_alpha");

    b.type = "sftp";
    b.endpoint = "user@ssh.example.com:/repos";
    EXPECT_EQ(remotePrefix(b), "user@ssh.example.com:/repos/team_alpha");

    b.type = "local";
    b.bucket = "/var/repos";
    EXPECT_EQ(remotePrefix(b), "/var/repos/team_alpha");
}

// ─── Ignore Rules Tests ───────────────────────────────────────────────────────

TEST(IgnoreTest, DefaultRulesAlwaysIncludeInternals) {
    TempRepo repo("ignore_default");
    auto rules = loadIgnoreRules();

    EXPECT_TRUE(isIgnored(".miniVCS/config", rules));
    EXPECT_TRUE(isIgnored(".miniVCS/objects/ab/123", rules));
    EXPECT_TRUE(isIgnored(".git/HEAD", rules));
    EXPECT_FALSE(isIgnored("src/main.cpp", rules));
}

TEST(IgnoreTest, CustomIgnoreFileRules) {
    TempRepo repo("ignore_custom");
    std::string ignoreContent =
        "# Comment line should be ignored\n"
        "\n"
        "node_modules\n"
        "*.tmp\n"
        "build/\n"
        "*.log\n";
    WriteFile(repo.path() / ".miniVCSignore", ignoreContent);

    auto rules = loadIgnoreRules();

    EXPECT_TRUE(isIgnored("node_modules/express/index.js", rules));
    EXPECT_TRUE(isIgnored("test.tmp", rules));
    EXPECT_TRUE(isIgnored("build/output.exe", rules));
    EXPECT_TRUE(isIgnored("server.log", rules));

    EXPECT_FALSE(isIgnored("src/index.js", rules));
    EXPECT_FALSE(isIgnored("README.md", rules));
    EXPECT_FALSE(isIgnored("logger.cpp", rules));
}
