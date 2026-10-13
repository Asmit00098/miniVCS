/**
 * minivcs.hpp
 * ─────────────────────────────────────────────────────────────────────────────
 * Public API and declarations for SecureVCS engine.
 * Included by tests and external modules.
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <filesystem>

namespace fs = std::filesystem;

// ── §1 & §2: SHA-256 ─────────────────────────────────────────────────────────
namespace sha256_impl {
    std::string hash(const std::string& data);
}

std::string sha256File(const fs::path& p);
std::string sha256Str(const std::string& s);

// ── §3: Security Guardrails ──────────────────────────────────────────────────
struct SecretMatch {
    std::string rule;
    std::string line;
    int lineNo;
};

bool matchesBlockedFilename(const std::string& filename);
std::vector<SecretMatch> scanFileForSecrets(const fs::path& filePath);

// ── §4: Object Store ─────────────────────────────────────────────────────────
std::string objectPath(const std::string& hash);
bool objectExists(const std::string& hash);
std::string storeBlob(const fs::path& srcFile);
std::string storeBlobStr(const std::string& content);
std::string readObject(const std::string& hash);

// ── §5: Myers Diff ───────────────────────────────────────────────────────────
struct DiffOp {
    char op;       // '+' add, '-' del, '=' keep
    std::string line;
    int origLine;  // for '-' and '=', 0-based line in original
    int newLine;   // for '+' and '=', 0-based line in new
};

std::vector<DiffOp> myersDiff(const std::vector<std::string>& A, const std::vector<std::string>& B);
std::string serializePatch(const std::vector<DiffOp>& ops);
std::string applyPatch(const std::vector<std::string>& original, const std::string& patchData);

// ── §6: Config ───────────────────────────────────────────────────────────────
std::map<std::string,std::string> readConfig();
void writeConfig(const std::map<std::string,std::string>& cfg);
std::string cfgGet(const std::string& key, const std::string& def = "");

// ── §7: Storage Backend ──────────────────────────────────────────────────────
struct StorageBackend {
    std::string type;      // s3 | gcs | azure | b2 | minio | sftp | local
    std::string bucket;    // bucket name or container or directory
    std::string endpoint;  // for minio/sftp
    std::string username;  // repo owner namespace
};

StorageBackend loadBackend();
std::string remotePrefix(const StorageBackend& b);
bool backendUpload(const StorageBackend& b, const std::string& localPath, const std::string& remotePath);
bool backendDownload(const StorageBackend& b, const std::string& remotePath, const std::string& localPath);

// ── §8: Tree & Commit Objects ────────────────────────────────────────────────
using Tree = std::map<std::string,std::string>;

std::string serializeTree(const Tree& t);
Tree parseTree(const std::string& data);

struct CommitMeta {
    std::string id;
    std::string treeHash;
    std::string parent;       // empty for root
    std::string branch;
    std::string author;
    std::string timestamp;
    std::string message;
    std::map<std::string,std::string> extras;
};

std::string escapeMeta(const std::string& s);
std::string unescapeMeta(const std::string& s);
std::string serializeCommit(const CommitMeta& c);
CommitMeta parseCommit(const std::string& data);

std::string currentBranch();
std::string headCommitId();
std::string getCommitMessage(const std::string& commitId);
void setHead(const std::string& commitId);
void saveCommitMeta(const CommitMeta& c);
std::optional<CommitMeta> loadCommitMeta(const std::string& id);

std::vector<std::string> loadIgnoreRules();
bool isIgnored(const std::string& relPath, const std::vector<std::string>& rules);

std::string ollamaCall(const std::string& prompt, const std::string& model = "tinyllama", double temp = 0.2, const std::vector<std::string>& stopTokens = {});
std::string generateCommitMessage(const std::vector<std::pair<std::string,std::string>>& changedFiles);
void searchCodebase(const std::string& query);

struct MergeResolution {
    std::string resolution;
    double confidence;  // 0.0 – 1.0
    std::string rationale;
};
std::string detectLanguageFromPath(const std::string& path);
std::string detectLanguageFromContent(const std::string& content);
std::string normalizeSourceCode(const std::string& code, const std::string& lang);
double lineLcsSimilarity(const std::vector<std::string>& A, const std::vector<std::string>& B);
double dualParentSimilarity(const std::vector<std::string>& cand, const std::vector<std::string>& V1, const std::vector<std::string>& V2);
std::string resolveConflictBySearch(const std::string& conflictContent);
std::vector<MergeResolution> resolveMergeSearch(const std::string& conflictContent);
std::vector<MergeResolution> resolveMergeHybrid(const std::string& conflictContent, const std::string& lang = "");
std::vector<MergeResolution> aiResolveMerge(const std::string& conflictContent, const std::string& lang = "");

// ── §11: Utilities ───────────────────────────────────────────────────────────
bool repoInitialized();
std::string getTimestamp();
Tree loadIndex();
void saveIndex(const Tree& t);
void printHelp();

// ── §12: CLI Commands ────────────────────────────────────────────────────────
void cmd_init();
void cmd_add(const std::string& path, bool forceAdd = false);
void cmd_status();
void cmd_commit(std::string message, bool autoMsg = false);
void dispatchCommit(const std::vector<std::string>& args);
void cmd_push();
void cmd_pull();
void cmd_log();
void cmd_checkout(const std::string& commitId);
void cmd_diff(const std::string& commitA, const std::string& commitB = "");
void cmd_clone(const std::string& remoteUser, const std::string& destDir = "");
void cmd_branch(const std::string& action, const std::string& name = "");
void cmd_merge_resolve(const std::string& conflictFile);
void cmd_remote(const std::string& action, const std::vector<std::string>& args);
void cmd_config(const std::string& action, const std::vector<std::string>& args);
void cmd_search(const std::string& query);
void cmd_reset();

