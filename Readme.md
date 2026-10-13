# SecureVCS 🔐

A **cloud-native version control system** where you store repository data in **your own storage** — not a central server. Built in modern C++ with a content-addressed object store (like Git), delta compression, security guardrails, local AI via Ollama, and AI-powered merge conflict resolution.

[![npm](https://img.shields.io/npm/v/minivcs?color=6366f1&style=flat-square)](https://www.npmjs.com/package/minivcs)
[![GitHub](https://img.shields.io/badge/GitHub-SecureVCS-181717?style=flat-square&logo=github)](https://github.com/GulshanJha00/minivcs)

---

## What's new in v2

| Feature | v1 | v2 |
|---|---|---|
| Storage engine | Full-file snapshots (copies everything) | **Content-addressed objects + deduplication** |
| Space usage | O(n × files) per commit | **O(changed_files) per commit** |
| Storage backends | AWS S3 only (hardcoded bucket) | **S3, GCS, Azure, B2, MinIO, SFTP, Local** |
| Secret scanning | None | **8+ patterns — blocks before upload** |
| AI commit messages | None | **Ollama Tiny (100% local, private)** |
| Codebase search | None | **Local Ollama natural language search** |
| Merge resolution | None | **Hybrid Meta-Resolver (LLM + Offline Search SBCR)** |
| Branch support | None | **Full create/switch/delete** |
| Diff engine | None | **Myers diff (line-level)** |

---

## Quick Start

```bash
npm install -g minivcs

minivcs init          # choose your storage backend
minivcs add .         # stage with security scanning
minivcs commit --ai   # AI writes the commit message
minivcs push          # upload to YOUR storage
```

---

## Storage Backends (BYOS)

Choose during `minivcs init` or switch at any time:

| Backend | Tool required |
|---|---|
| **AWS S3** | `aws` CLI |
| **Google Cloud Storage** | `gsutil` |
| **Azure Blob** | `az` CLI |
| **Backblaze B2** | `b2` CLI |
| **MinIO / self-hosted** | `aws` CLI + endpoint |
| **SFTP server** | `rsync` |
| **Local directory** | none |

```bash
# Switch to MinIO
minivcs config set backend minio
minivcs config set endpoint http://localhost:9000
minivcs config set bucket my-bucket
```

---

## Security Guardrails

Before any file is staged, SecureVCS checks:
1. **Blocked filenames**: `.env`, `*.pem`, `*.key`, `id_rsa`, `secrets.yml`, etc.
2. **Secret patterns**: AWS keys, API tokens, private key headers, hardcoded passwords, DB URLs

```
[BLOCKED] .env — sensitive filename
[BLOCKED] config.js — [AWS Access Key] AKIAIOSFODNN7EXAMPLE
```

Use `--force` to override (not recommended).

---

## AI Features

### Ollama (local, private)
```bash
ollama pull tinyllama
minivcs config set ollama_model tinyllama

minivcs search "where is JWT validated"   # local codebase search
minivcs commit --ai                        # auto-generate commit message
```

### Merge Conflict Resolution (Hybrid Meta-Resolver)

Synthesizes both **Merge-Bench** (*Schesch & Ernst, May 2026*) and **Search-Based Engineering** (*Campos Junior & Murta, arXiv:2605.16646, May 2026*). Works online with cloud AI (Gemini, OpenAI) or **100% offline** via combinatorial Random Restart Hill Climbing (SBCR):

```bash
# Optional: configure cloud AI (Google Gemini or OpenAI)
minivcs config set ai_api_url https://generativelanguage.googleapis.com/v1beta/openai/chat/completions
minivcs config set ai_model gemini-3.5-flash
minivcs config set ai_api_key YOUR_GEMINI_KEY

# Resolve any merge conflict (works with or without an API key!)
minivcs merge-resolve src/auth.js

# Output:
# ═══ AI Merge Conflict Resolutions ═══
# Option 1 — Confidence: 95% (Semantic Synthesis / SBCR Rescue)
# Option 2 — Confidence: 88% (Search-Based Software Engineering - SBCR)
# Option 3 — Confidence: 70% (Conservative Branch Fallback)
# Apply which option? [1-3/n to skip]:
```

---

## All Commands

```
Core:     init, add, commit, status, log, checkout, diff, reset
Remote:   push, pull, clone
Branches: branch list/create/switch/delete
AI:       search, merge-resolve
Config:   config list/get/set, remote show/set
```

---

## Build from Source

```bash
# macOS / Linux with OpenSSL
g++ -std=c++17 -O2 main.cpp -lssl -lcrypto -o miniVCS

# Without OpenSSL (built-in SHA-256)
g++ -std=c++17 -O2 -DMINIVCS_NO_OPENSSL main.cpp -o miniVCS

# Build script (all platforms)
./build.sh all
```

---

## Testing & Quality Assurance

SecureVCS includes a test suite covering unit, security, algorithmic, and end-to-end integration workflows.

### Running Tests

```bash
# PowerShell (Windows)
.\run_tests.ps1

# Or via npm
npm test

# Linux / macOS / Git Bash
./run_tests.sh

# Or via CMake + CTest
mkdir build && cd build
cmake -DBUILD_TESTS=ON ..
cmake --build .
ctest --output-on-failure
```

### Test Coverage Matrix (79 Tests, 100% Pass Rate)

| Module | Test Target | Key Assertions Covered |
|---|---|---|
| **SHA-256** | `tests/unit/test_sha256.cpp` | NIST CAVP vectors (empty string, short, 56B multi-block), determinism, avalanche effect, file/string hash equivalence |
| **Security Guardrails** | `tests/unit/test_security.cpp` | Filename blocklist (`.env*`, `*.pem`, `*.key`, `id_rsa`), case-insensitivity, secret scanning (AWS, Private Keys, GitHub tokens, Slack tokens, DB URIs) |
| **Myers Diff** | `tests/unit/test_diff.cpp` | Identical file zero-delta, insertions, deletions, middle-line modifications, patch serialization |
| **Object Store** | `tests/unit/test_objects.cpp` | Two-level fan-out (`objects/<xx>/<rest>`), blob storage, content deduplication, exact byte-for-byte read |
| **Serialization** | `tests/unit/test_serialization.cpp` | Tree object roundtrip, commit metadata roundtrip, root commit empty parent, disk persistence |
| **Configuration** | `tests/unit/test_config.cpp` | Read/write config, default fallbacks, multi-cloud URL formats (S3, GCS, Azure, B2, MinIO, SFTP, Local), ignore rules |
| **AI & Merge Resolution** | `tests/unit/test_ai.cpp` | Ollama offline handling, commit message synthesis, language detection (11 languages), comment/docstring normalization, Gestalt LCS similarity, dual-parent fitness (rho = 0.791), partial-order search, SBCR offline rescue |
| **Integration CLI** | `tests/integration/test_cli.cpp` | Full workflow: `init` → `add` (guardrail blocks & overrides) → `commit` → `branch` → `reset` → `checkout` (version recovery) |

---

## Documentation

📖 [Full Docs](https://mini-vcs.vercel.app/docs)

---

Built by [Gulshan Kumar](https://github.com/GulshanJha00) · MIT License