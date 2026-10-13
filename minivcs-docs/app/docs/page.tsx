"use client";

import { useState, useEffect, useRef } from "react";

// ─── Icons ────────────────────────────────────────────────────────────────────

function GitHubIcon({ className = "" }: { className?: string }) {
  return (
    <svg className={className} viewBox="0 0 24 24" fill="currentColor">
      <path d="M12 0C5.37 0 0 5.37 0 12c0 5.31 3.435 9.795 8.205 11.385.6.105.825-.255.825-.57 0-.285-.015-1.23-.015-2.235-3.015.555-3.795-.735-4.035-1.41-.135-.345-.72-1.41-1.23-1.695-.42-.225-1.02-.78-.015-.795.945-.015 1.62.87 1.845 1.23 1.08 1.815 2.805 1.305 3.495.99.105-.78.42-1.305.765-1.605-2.67-.3-5.46-1.335-5.46-5.925 0-1.305.465-2.385 1.23-3.225-.12-.3-.54-1.53.12-3.18 0 0 1.005-.315 3.3 1.23.96-.27 1.98-.405 3-.405s2.04.135 3 .405c2.295-1.56 3.3-1.23 3.3-1.23.66 1.65.24 2.88.12 3.18.765.84 1.23 1.905 1.23 3.225 0 4.605-2.805 5.625-5.475 5.925.435.375.81 1.095.81 2.22 0 1.605-.015 2.895-.015 3.3 0 .315.225.69.825.57A12.02 12.02 0 0 0 24 12c0-6.63-5.37-12-12-12z" />
    </svg>
  );
}

function NpmIcon({ className = "" }: { className?: string }) {
  return (
    <svg className={className} viewBox="0 0 24 24" fill="currentColor">
      <path d="M0 7.334v8h6.666v1.332H12v-1.332h12v-8H0zm6.666 6.664H5.334v-4H3.999v4H1.335V8.667h5.331v5.331zm4 0v1.336H8.001V8.667h5.334v5.332h-2.669v-.001zm12.001 0h-1.33v-4h-1.336v4h-1.335v-4h-1.33v4h-2.671V8.667h8.002v5.331z" />
    </svg>
  );
}

function ChevronIcon({ open }: { open: boolean }) {
  return (
    <svg
      className={`w-4 h-4 text-slate-500 transition-transform duration-200 ${open ? "rotate-90" : ""}`}
      fill="none" viewBox="0 0 24 24" stroke="currentColor" strokeWidth={2}
    >
      <path strokeLinecap="round" strokeLinejoin="round" d="M9 5l7 7-7 7" />
    </svg>
  );
}

function CopyIcon() {
  return (
    <svg className="w-3 h-3" fill="none" viewBox="0 0 24 24" stroke="currentColor" strokeWidth={2}>
      <path strokeLinecap="round" strokeLinejoin="round" d="M8 16H6a2 2 0 01-2-2V6a2 2 0 012-2h8a2 2 0 012 2v2m-6 12h8a2 2 0 002-2v-8a2 2 0 00-2-2h-8a2 2 0 00-2 2v8a2 2 0 002 2z" />
    </svg>
  );
}

// ─── Terminal Component ───────────────────────────────────────────────────────

function Terminal({ children, label = "bash" }: { children: string; label?: string }) {
  const [copied, setCopied] = useState(false);

  const handleCopy = () => {
    navigator.clipboard.writeText(children).then(() => {
      setCopied(true);
      setTimeout(() => setCopied(false), 1500);
    });
  };

  // Syntax highlight
  const highlight = (text: string) => {
    const lines = text.split("\n");
    return lines.map((line, i) => {
      if (line.startsWith("#")) {
        return <span key={i} className="block text-slate-600">{line}{"\n"}</span>;
      }
      if (line.startsWith("[OK]")) {
        return <span key={i} className="block"><span className="text-emerald-400">{line.substring(0, 4)}</span>{line.substring(4)}{"\n"}</span>;
      }
      if (line.startsWith("[BLOCKED]") || line.startsWith("[!]")) {
        return <span key={i} className="block"><span className="text-red-400">{line.substring(0, line.indexOf("]") + 1)}</span>{line.substring(line.indexOf("]") + 1)}{"\n"}</span>;
      }
      if (line.startsWith("[WARN]")) {
        return <span key={i} className="block"><span className="text-yellow-400">{line.substring(0, 6)}</span>{line.substring(6)}{"\n"}</span>;
      }
      // Command line
      if (line.startsWith("minivcs ") || line.startsWith("$ ")) {
        const parts = line.split(" ");
        return (
          <span key={i} className="block">
            <span className="text-sky-300">{parts[0]}</span>{" "}
            <span className="text-indigo-300">{parts[1]}</span>{" "}
            <span className="text-emerald-300">{parts.slice(2).join(" ")}</span>
            {"\n"}
          </span>
        );
      }
      return <span key={i} className="block">{line}{"\n"}</span>;
    });
  };

  return (
    <div className="terminal group my-4">
      <div className="terminal-bar">
        <div className="terminal-dot" style={{ background: "#ff5f57" }} />
        <div className="terminal-dot" style={{ background: "#febc2e" }} />
        <div className="terminal-dot" style={{ background: "#28c840" }} />
        <span className="ml-2 text-slate-500 text-xs mono flex-1">{label}</span>
        <button onClick={handleCopy} className="copy-btn flex items-center gap-1">
          <CopyIcon />
          {copied ? "copied!" : "copy"}
        </button>
      </div>
      <div className="terminal-body">{highlight(children)}</div>
    </div>
  );
}

// ─── Accordion Card ───────────────────────────────────────────────────────────

interface CardProps {
  badge?: string;
  badgeColor?: string;
  icon?: string;
  name: string;
  shortDesc: string;
  usage: string | string[];
  description: string;
  steps?: string[];
  output?: string;
  note?: string;
  warning?: string;
}

function AccordionCard({
  badge, badgeColor = "badge-blue", icon = "⚡", name, shortDesc,
  usage, description, steps, output, note, warning
}: CardProps) {
  const [open, setOpen] = useState(false);
  const usageStr = Array.isArray(usage) ? usage.join("\n") : usage;

  return (
    <div className="glass rounded-xl overflow-hidden mb-3">
      <button
        id={`accordion-${name.replace(/\s/g, "-")}`}
        onClick={() => setOpen(o => !o)}
        className="accordion-header w-full flex items-center gap-3 px-5 py-4 text-left"
        aria-expanded={open}
      >
        <span className="text-lg">{icon}</span>
        <div className="flex-1 min-w-0">
          <div className="flex items-center gap-2 mb-0.5 flex-wrap">
            <span className="mono text-sm font-semibold text-white">{name}</span>
            {badge && <span className={`badge ${badgeColor}`}>{badge}</span>}
          </div>
          <span className="text-xs text-slate-500 truncate block">{shortDesc}</span>
        </div>
        <ChevronIcon open={open} />
      </button>

      {open && (
        <div className="px-5 pb-5 border-t border-white/5 space-y-4 pt-4">
          <Terminal>{usageStr}</Terminal>
          <p className="text-sm text-slate-400 leading-relaxed">{description}</p>

          {warning && (
            <div className="flex gap-2 p-3 rounded-lg bg-yellow-400/5 border border-yellow-400/20">
              <span className="text-yellow-400 text-sm">⚠</span>
              <p className="text-sm text-yellow-300/80">{warning}</p>
            </div>
          )}

          {steps && steps.length > 0 && (
            <div>
              <p className="section-label mb-3">Internal flow</p>
              <div className="space-y-1">
                {steps.map((step, i) => (
                  <div key={i} className="flex items-start gap-3 py-2 px-3 rounded-lg hover:bg-white/[0.02]">
                    <span className="mono text-xs text-indigo-400 w-5 shrink-0 pt-0.5">{String(i + 1).padStart(2, "0")}</span>
                    <span className="text-sm text-slate-400">{step}</span>
                  </div>
                ))}
              </div>
            </div>
          )}

          {output && <Terminal label="output">{output}</Terminal>}

          {note && (
            <p className="text-xs text-slate-500 italic border-l-2 border-indigo-800 pl-3">
              {note}
            </p>
          )}
        </div>
      )}
    </div>
  );
}

// ─── Navbar ───────────────────────────────────────────────────────────────────

function Navbar() {
  const [scrolled, setScrolled] = useState(false);
  useEffect(() => {
    const handler = () => setScrolled(window.scrollY > 20);
    window.addEventListener("scroll", handler);
    return () => window.removeEventListener("scroll", handler);
  }, []);

  return (
    <header
      className={`sticky top-0 z-50 transition-all duration-300 ${
        scrolled
          ? "border-b border-white/[0.06] bg-[#060912]/90 backdrop-blur-xl"
          : "bg-transparent"
      }`}
    >
      <div className="max-w-6xl mx-auto px-4 sm:px-6 h-14 flex items-center justify-between">
        <div className="flex items-center gap-2.5">
          <div className="w-6 h-6 rounded-md bg-indigo-600 flex items-center justify-center">
            <span className="text-white text-xs font-bold">S</span>
          </div>
          <span className="mono text-sm font-semibold text-white">SecureVCS</span>
          <span className="mono text-[10px] text-slate-600 border border-white/[0.06] rounded-full px-2 py-0.5">v2.0</span>
        </div>

        <nav className="flex items-center gap-1">
          {[
            { label: "Docs", href: "#commands" },
            { label: "Security", href: "#security" },
            { label: "AI", href: "#ai" },
          ].map(({ label, href }) => (
            <a
              key={href}
              href={href}
              className="px-3 py-1.5 text-xs text-slate-400 hover:text-white hover:bg-white/[0.05] rounded-lg transition-all"
            >
              {label}
            </a>
          ))}
          <a
            href="https://github.com/GulshanJha00/minivcs"
            target="_blank"
            rel="noopener noreferrer"
            className="flex items-center gap-1.5 ml-2 px-3 py-1.5 text-xs text-slate-300 hover:text-white bg-white/[0.05] hover:bg-white/[0.09] border border-white/[0.07] rounded-lg transition-all"
          >
            <GitHubIcon className="w-3.5 h-3.5" />
            GitHub
          </a>
        </nav>
      </div>
    </header>
  );
}

// ─── Feature Card ─────────────────────────────────────────────────────────────

function FeatureCard({
  icon, title, desc, badge, badgeColor = "badge-blue"
}: {
  icon: string; title: string; desc: string; badge?: string; badgeColor?: string;
}) {
  return (
    <div className="glass glass-hover rounded-xl p-5">
      <div className="feature-icon mb-3" style={{ background: "rgba(99,102,241,0.12)" }}>
        {icon}
      </div>
      <div className="flex items-center gap-2 mb-1.5">
        <h3 className="text-sm font-semibold text-white">{title}</h3>
        {badge && <span className={`badge ${badgeColor}`}>{badge}</span>}
      </div>
      <p className="text-xs text-slate-500 leading-relaxed">{desc}</p>
    </div>
  );
}

// ─── Backend Card ─────────────────────────────────────────────────────────────

function BackendCard({
  icon, name, desc, cmd, color
}: {
  icon: string; name: string; desc: string; cmd: string; color: string;
}) {
  return (
    <div className="glass glass-hover rounded-xl p-4">
      <div className="flex items-center gap-2.5 mb-2">
        <span className="text-xl">{icon}</span>
        <span className="text-sm font-semibold text-white">{name}</span>
        <span className={`mono text-xs ml-auto`} style={{ color }}>{cmd}</span>
      </div>
      <p className="text-xs text-slate-500">{desc}</p>
    </div>
  );
}

// ─── Confidence Bar ───────────────────────────────────────────────────────────

function ConfidenceBar({ pct, label }: { pct: number; label: string }) {
  const color = pct >= 80 ? "#34d399" : pct >= 50 ? "#fbbf24" : "#f87171";
  return (
    <div className="flex items-center gap-3">
      <span className="text-xs text-slate-400 w-28 shrink-0">{label}</span>
      <div className="progress-bar flex-1">
        <div className="progress-fill" style={{ width: `${pct}%`, background: color }} />
      </div>
      <span className="mono text-xs" style={{ color }}>{pct}%</span>
    </div>
  );
}

// ─── Divider ──────────────────────────────────────────────────────────────────

function Divider() {
  return <hr className="divider" />;
}

function SectionLabel({ children }: { children: React.ReactNode }) {
  return <p className="section-label">{children}</p>;
}

// ─── Page ─────────────────────────────────────────────────────────────────────

export default function Docs() {
  return (
    <>
      <div className="bg-grid hero-glow min-h-screen">
        <Navbar />

        <div className="max-w-4xl mx-auto px-4 sm:px-6">

          {/* ── Hero ─────────────────────────────────────────────────────── */}
          <section id="intro" className="pt-20 pb-16 fade-up">
            <div className="flex flex-wrap gap-2 mb-6">
              {["C++17", "Delta Compression", "Multi-Backend BYOS", "AI Merge Resolution", "Secret Scanner", "Ollama Local Search"].map(tag => (
                <span key={tag} className="token">{tag}</span>
              ))}
            </div>

            <h1 className="text-4xl sm:text-6xl font-bold mb-4 leading-tight">
              <span style={{ background: "linear-gradient(135deg, #818cf8 0%, #a78bfa 50%, #c4b5fd 100%)", WebkitBackgroundClip: "text", WebkitTextFillColor: "transparent" }}>
                SecureVCS
              </span>
            </h1>

            <p className="text-slate-400 text-base sm:text-lg leading-relaxed max-w-2xl mb-8">
              A cloud-native version control system where <strong className="text-white">you own your storage</strong>.
              Store repository data in your own S3, GCS, Azure, MinIO, or any S3-compatible backend.
              Features delta compression, security guardrails, local AI search via Ollama, and
              AI-powered merge conflict resolution.
            </p>

            <div className="flex flex-wrap gap-3 mb-10">
              <a
                href="https://github.com/GulshanJha00/minivcs"
                target="_blank"
                rel="noopener noreferrer"
                id="cta-github"
                className="inline-flex items-center gap-2 px-5 py-2.5 rounded-xl bg-white text-slate-900 text-sm font-semibold hover:bg-slate-100 transition-colors"
              >
                <GitHubIcon className="w-4 h-4" />
                View on GitHub
              </a>
              <a
                href="https://www.npmjs.com/package/minivcs"
                target="_blank"
                rel="noopener noreferrer"
                id="cta-npm"
                className="inline-flex items-center gap-2 px-5 py-2.5 rounded-xl border border-white/[0.08] text-slate-300 text-sm font-semibold hover:bg-white/[0.05] hover:text-white transition-all"
              >
                <NpmIcon className="w-4 h-4 text-red-400" />
                npm package
              </a>
            </div>

            <Terminal label="quick start">{`# Install
npm install -g minivcs

# Initialize with your own storage
minivcs init
# → Choose backend: S3, GCS, Azure, MinIO, SFTP, local...

# Stage files (with automatic secret scanning)
minivcs add .

# Commit (or let AI write the message)
minivcs commit --ai

# Push to your personal storage bucket
minivcs push`}</Terminal>
          </section>

          {/* ── Feature grid ─────────────────────────────────────────────── */}
          <section id="features" className="mb-16 fade-up-2">
            <SectionLabel>What&apos;s new in v2</SectionLabel>
            <h2 className="text-2xl sm:text-3xl font-bold text-white mb-6">Everything, upgraded</h2>
            <div className="grid grid-cols-1 sm:grid-cols-2 lg:grid-cols-3 gap-3">
              <FeatureCard
                icon="🗜️" title="Delta Compression" badge="New" badgeColor="badge-green"
                desc="Git-style content-addressed object store. Only changed lines are stored — no full file copies per commit. Myers diff algorithm for line-level diffs."
              />
              <FeatureCard
                icon="☁️" title="Multi-Backend Storage" badge="New" badgeColor="badge-green"
                desc="Bring your own storage: AWS S3, Google Cloud Storage, Azure Blob, Backblaze B2, MinIO (self-hosted), SFTP, or a local directory."
              />
              <FeatureCard
                icon="🔐" title="Security Guardrails" badge="New" badgeColor="badge-red"
                desc="Automatically blocks .env files, .pem keys, hardcoded API keys, AWS secrets, tokens, and database URLs before they ever reach your remote storage."
              />
              <FeatureCard
                icon="🤖" title="Ollama AI Search" badge="New" badgeColor="badge-purple"
                desc="Search your codebase with Ollama Tiny — completely local, no data leaves your machine. Also auto-generates commit messages from your staged changes."
              />
              <FeatureCard
                icon="⚔️" title="AI Merge Resolution" badge="New" badgeColor="badge-purple"
                desc="Paste your merge conflict; get 3 ranked resolutions with confidence scores from a custom AI API (OpenAI-compatible). Pick the best one with one keystroke."
              />
              <FeatureCard
                icon="🌿" title="Branch Support" badge="New" badgeColor="badge-blue"
                desc="Full branch management: create, switch, delete. Commit history chains via parent pointers — the full Git-style DAG model."
              />
            </div>
          </section>

          <Divider />

          {/* ── Install ──────────────────────────────────────────────────── */}
          <section id="install" className="mb-16">
            <SectionLabel>Installation</SectionLabel>
            <h2 className="text-2xl font-bold text-white mb-4">Get started</h2>
            <Terminal>{`npm install -g minivcs`}</Terminal>
            <p className="text-sm text-slate-500 mt-3">
              Installs the Node.js CLI wrapper that shells out to the compiled C++ binary.
              Requires Node.js. Also install the CLI tool for your chosen backend (AWS CLI, gsutil, az, b2, etc.).
            </p>

            <div className="glass rounded-xl p-5 mt-5">
              <p className="section-label mb-4">Prerequisites by backend</p>
              <div className="space-y-2 text-sm">
                {[
                  ["AWS S3",    "aws cli + aws configure"],
                  ["GCS",       "google-cloud-sdk (gsutil)"],
                  ["Azure",     "azure-cli (az login)"],
                  ["Backblaze", "b2 cli + b2 authorize-account"],
                  ["MinIO",     "aws cli + custom --endpoint-url"],
                  ["SFTP",      "rsync + SSH key access"],
                  ["Local",     "none"],
                ].map(([backend, req]) => (
                  <div key={backend} className="flex items-center gap-3">
                    <span className="text-white w-24 shrink-0">{backend}</span>
                    <span className="mono text-xs text-indigo-400">{req}</span>
                  </div>
                ))}
              </div>
            </div>
          </section>

          <Divider />

          {/* ── Storage Backends ─────────────────────────────────────────── */}
          <section id="backends" className="mb-16">
            <SectionLabel>Storage</SectionLabel>
            <h2 className="text-2xl font-bold text-white mb-2">Bring Your Own Storage (BYOS)</h2>
            <p className="text-sm text-slate-400 mb-6 leading-relaxed">
              SecureVCS never uses a central server. Your commits go directly to <strong className="text-white">your</strong> storage bucket.
              Choose during <code className="mono text-indigo-300">minivcs init</code>, or change at any time with{" "}
              <code className="mono text-indigo-300">minivcs config set backend &lt;type&gt;</code>.
            </p>

            <div className="grid grid-cols-1 sm:grid-cols-2 gap-3 mb-6">
              <BackendCard icon="🟠" name="AWS S3" desc="Amazon Simple Storage Service. The default. Uses aws s3 cp/sync." cmd="backend=s3" color="#fb923c" />
              <BackendCard icon="🔵" name="Google Cloud Storage" desc="GCS buckets via gsutil. Works with service accounts." cmd="backend=gcs" color="#60a5fa" />
              <BackendCard icon="🔷" name="Azure Blob Storage" desc="Microsoft Azure containers via az cli." cmd="backend=azure" color="#818cf8" />
              <BackendCard icon="🟡" name="Backblaze B2" desc="Affordable S3-compatible cloud storage via b2 cli." cmd="backend=b2" color="#fbbf24" />
              <BackendCard icon="🐋" name="MinIO (Self-Hosted)" desc="Run your own S3-compatible object store. Full data sovereignty." cmd="backend=minio" color="#34d399" />
              <BackendCard icon="📡" name="SFTP / SSH Server" desc="Any server you have SSH access to via rsync." cmd="backend=sftp" color="#a78bfa" />
              <BackendCard icon="📁" name="Local Directory" desc="A local filesystem path. For testing or air-gapped systems." cmd="backend=local" color="#94a3b8" />
            </div>

            <Terminal label="switch backend">{`# Switch to MinIO self-hosted
minivcs config set backend minio
minivcs config set endpoint http://localhost:9000
minivcs config set bucket my-repo-bucket

# Switch to GCS
minivcs config set backend gcs
minivcs config set bucket my-gcs-bucket`}</Terminal>
          </section>

          <Divider />

          {/* ── Commands ─────────────────────────────────────────────────── */}
          <section id="commands" className="mb-16">
            <SectionLabel>Commands</SectionLabel>
            <h2 className="text-2xl font-bold text-white mb-6">Command reference</h2>

            <AccordionCard
              icon="🚀" name="minivcs init" badge="Core" shortDesc="Initialize repository + choose storage backend"
              usage="minivcs init"
              description="Initializes the .miniVCS metadata directory in the current folder. Prompts for a username and lets you pick a storage backend. Creates the object store, branch refs, HEAD pointer, and a default .miniVCSignore."
              steps={[
                "Check whether .miniVCS already exists",
                "Prompt for username",
                "Display storage backend menu (S3 / GCS / Azure / B2 / MinIO / SFTP / Local)",
                "Collect backend-specific parameters (bucket, endpoint…)",
                "Create .miniVCS/objects/, commits/, refs/heads/ directories",
                "Write config file with all settings",
                "Write HEAD → refs/heads/main",
                "Create .miniVCSignore with safe defaults",
              ]}
              note="Your storage credentials are never stored inside .miniVCS — the system relies on the credentials already configured in your cloud CLI tools."
            />

            <AccordionCard
              icon="📂" name="minivcs add" badge="Core" shortDesc="Stage files with automatic secret scanning"
              usage={["minivcs add .", "minivcs add src/", "minivcs add README.md", "minivcs add . --force   # override security blocks (not recommended)"]}
              description="Stages files for the next commit. Each file is SHA-256 hashed and stored as a content-addressed blob in .miniVCS/objects/. Identical content is stored only once across all commits (deduplication)."
              steps={[
                "Load ignore rules from .miniVCSignore",
                "For each file: check if filename matches blocked list (.env, .pem, .key…)",
                "Scan file content for secrets using 8+ regex patterns (AWS keys, API tokens, DB URLs…)",
                "Block the file with a descriptive error if any issue found",
                "Hash file content with SHA-256",
                "Write blob to .miniVCS/objects/<xx>/<rest> (content-addressed)",
                "Update .miniVCS/INDEX with hash → path mapping",
              ]}
              warning="By default, .env files and files containing hardcoded secrets are refused. Use --force only if you understand the security implications."
              output={`[OK] 12 file(s) staged
[BLOCKED] .env — sensitive filename. Use --force to override.
[BLOCKED] config.js — potential secret detected:
  Line 3: [AWS Access Key] AKIAIOSFODNN7EXAMPLE`}
              note="Identical files across commits share a single object — zero space wasted on repeated content."
            />

            <AccordionCard
              icon="💾" name="minivcs commit" badge="Core" shortDesc="Create a delta snapshot + push to remote"
              usage={["minivcs commit -m \"added auth module\"", "minivcs commit \"added auth module\"", "minivcs commit --ai   # auto-generate message with Ollama"]}
              description="Snapshots all staged files into a tree object (a manifest of path → blob hash), writes a commit object with metadata, advances the branch pointer, then automatically pushes new objects and the commit to your remote storage."
              steps={[
                "Verify staging area (INDEX) is not empty",
                "If --ai: call Ollama to generate a commit message from staged file list",
                "Build a tree object: map of relPath → blobHash",
                "Store tree as a content-addressed blob",
                "Write commit object: id, tree, parent, branch, author, timestamp, message",
                "Advance branch ref in .miniVCS/refs/heads/<branch>",
                "Clear staging index",
                "Push new objects + commit to remote storage backend",
              ]}
              output={`[OK] Committed: commit_20261013_142301
    feat: add JWT authentication middleware

Pushing to remote storage...
[OK] Pushed to s3://my-bucket`}
              note="Only new blob objects are uploaded — previously committed files are already in the remote object store."
            />

            <AccordionCard
              icon="🔄" name="minivcs checkout" badge="Core" shortDesc="Restore working directory to a commit"
              usage="minivcs checkout commit_20261013_142301"
              description="Restores the working directory to the exact state of a chosen commit. Reads the commit's tree object, fetches all blobs, removes current files, and reconstructs from the object store."
              steps={[
                "Abort if staging area has unsaved changes",
                "Look up commit metadata locally",
                "If not found, run pull to fetch from remote",
                "Read tree object → get list of files and their hashes",
                "Delete all files in working directory (except .miniVCS/)",
                "For each file in tree: read blob from object store, write to disk",
                "Update HEAD to point to checked-out commit",
              ]}
              warning="This overwrites your working directory. Commit or stash any changes first."
            />

            <AccordionCard
              icon="🌿" name="minivcs branch" badge="Branching" shortDesc="Create, switch, list, and delete branches"
              usage={["minivcs branch list", "minivcs branch create feature/auth", "minivcs branch switch feature/auth", "minivcs branch delete feature/auth"]}
              description="Branches are lightweight pointers stored in .miniVCS/refs/heads/. Creating a branch costs nothing — it just writes the current commit ID to a file. Switching updates HEAD."
              steps={[
                "list: iterate refs/heads/ directory, mark current branch with *",
                "create: write current HEAD commit to refs/heads/<name>",
                "switch: update .miniVCS/HEAD → ref: refs/heads/<name>",
                "delete: remove refs/heads/<name> file",
              ]}
              output={`* main
  feature/auth
  hotfix/login-bug`}
            />

            <AccordionCard
              icon="📊" name="minivcs diff" badge="Core" shortDesc="Line-level Myers diff between commits"
              usage={["minivcs diff                     # HEAD vs staged", "minivcs diff HEAD               # HEAD vs staged", "minivcs diff commit_A commit_B  # two commits"]}
              description="Uses the Myers diff algorithm to show exactly which lines changed between two commits (or between HEAD and your staged changes). Displayed with + green additions and - red deletions."
              output={`diff HEAD → STAGED: src/auth.js
+   const jwt = require('jsonwebtoken');
+   const SECRET = process.env.JWT_SECRET;
-   const SECRET = "hardcoded_bad";`}
            />

            <AccordionCard
              icon="📜" name="minivcs log" badge="Core" shortDesc="Walk the commit chain and display history"
              usage="minivcs log"
              description="Walks the linked list of commits from HEAD backwards through parent pointers, showing up to 50 commits. Displays commit ID, author, date, branch, and message."
              output={`commit commit_20261013_142301
Author:  masmi
Date:    20261013_142301
Branch:  main
    feat: add JWT authentication middleware

commit commit_20261012_091045
Author:  masmi
Date:    20261012_091045
Branch:  main
    init: project scaffold`}
            />

            <AccordionCard
              icon="📋" name="minivcs status" badge="Core" shortDesc="Show staged changes vs HEAD"
              usage="minivcs status"
              description="Compares the current staging index against the HEAD tree to categorize each file as new, modified, unchanged, or deleted."
              output={`On branch main
HEAD: a3f7c92...

Staged changes:
  + (new)       src/auth/jwt.js
  M (modified)  README.md
  = (unchanged) package.json`}
            />

            <AccordionCard
              icon="🔁" name="minivcs push / pull" badge="Remote" shortDesc="Sync objects and commits with remote storage"
              usage={["minivcs push   # upload all local objects + commits to remote", "minivcs pull   # download remote objects + commits to local"]}
              description="Push and pull sync the .miniVCS/objects, commits, and refs directories with your remote storage backend. Commit already pushes automatically — use push/pull for explicit full syncs."
              steps={[
                "push: upload objects/, commits/, refs/ to remote prefix",
                "pull: download remote objects/, commits/, refs/ to local",
                "Backend-specific tools handle deduplication (S3/GCS sync only transfer missing files)",
              ]}
            />

            <AccordionCard
              icon="📥" name="minivcs clone" badge="Remote" shortDesc="Clone any user's repository from remote storage"
              usage={["minivcs clone alice           # clone into ./alice/", "minivcs clone alice my-dir   # clone into ./my-dir/"]}
              description="Downloads all commits and objects from the remote storage namespace of a given user, then checks out their latest commit."
              steps={[
                "Create destination directory with .miniVCS/ structure",
                "Download objects/, commits/, refs/ from remote",
                "Find the latest commit by lexicographic ordering",
                "Read tree object, restore all files to working directory",
                "Write .miniVCS/config and HEAD for the cloned repo",
              ]}
            />

            <AccordionCard
              icon="🔧" name="minivcs config" badge="Config" shortDesc="Read and write repository configuration"
              usage={["minivcs config list", "minivcs config get backend", "minivcs config set ai_api_key sk-...", "minivcs config set backend minio", "minivcs config set endpoint http://localhost:9000"]}
              description="Reads and writes .miniVCS/config. All settings (backend, bucket, endpoint, Ollama URL/model, AI API key/URL/model) are stored here."
              output={`username=masmi
backend=s3
bucket=my-vcs-bucket
ollama_model=tinyllama
ollama_url=http://localhost:11434
ai_api_key=sk-...
ai_model=gpt-4o-mini
ai_api_url=https://api.openai.com/v1/chat/completions`}
            />

            <AccordionCard
              icon="🗑️" name="minivcs reset" badge="Core" shortDesc="Clear the staging area"
              usage="minivcs reset"
              description="Clears the staging index (removes all staged file mappings). Does not delete blobs from the object store or affect any commits."
            />

            <AccordionCard
              icon="🔎" name="minivcs search" badge="AI" badgeColor="badge-purple" shortDesc="Local natural language codebase search"
              usage={'minivcs search "where is JWT validated"'}
              description="Queries your local Ollama instance with your query against non-binary repository files. 100% private — your code never leaves your workstation. Automatically filters binary payloads and respects .miniVCSignore rules."
              output={`[Ollama] Searching codebase (locally, no data leaves machine)...
[src/auth.js:14] JWT token signature verified with verifyToken() middleware.`}
            />

            <AccordionCard
              icon="⚔️" name="minivcs merge-resolve" badge="Hybrid" badgeColor="badge-purple" shortDesc="AI & Search-Based conflict resolution"
              usage={["minivcs merge-resolve src/auth.js", "minivcs merge-resolve service.py"]}
              description="Resolves merge conflicts using an intelligent Hybrid Meta-Resolver combining Generative AI (Merge-Bench standard) with Search-Based Software Engineering (SBCR, arXiv:2605.16646). Works online with cloud LLMs (Gemini, OpenAI) or 100% offline via combinatorial Random Restart Hill Climbing with dual-parent similarity scoring."
              steps={[
                "Parse <<<<<<<, =======, and >>>>>>> markers in the conflict file",
                "Detect language and normalize source code syntax across 11 programming languages",
                "Generate partial-order preserving combinatorial candidate (SBCR) and evaluate dual-parent LCS fitness",
                "If AI key configured: query LLM with Merge-Bench semantic intent prompt",
                "If LLM fails, times out, or produces empty/truncated output: automatically rescue with SBCR candidate",
                "Present ranked resolutions with confidence scores and rationales, writing selected option to file",
              ]}
              output={`[AI] Analyzing merge conflict in service.py...
═══ AI Merge Conflict Resolutions ═══
Option 1 — Confidence: 95%
Rationale: Semantic synthesis: reconciled concurrent modifications into unified implementation.
─────────────────────────────────
[resolved code shown here]

Option 2 — Confidence: 88%
Rationale: Search-based combinatorial optimization (SBCR): maximizes dual-parent similarity (82%) while strictly preserving relative line partial order.
─────────────────────────────────
[search-based combination shown here]

Apply which option? [1-3/n to skip]:`}
              note="Works completely offline without an API key by automatically activating the Search-Based Software Engineering (SBCR) optimizer."
            />
          </section>

          <Divider />

          {/* ── Security ─────────────────────────────────────────────────── */}
          <section id="security" className="mb-16">
            <SectionLabel>Security</SectionLabel>
            <h2 className="text-2xl font-bold text-white mb-2">Built-in guardrails</h2>
            <p className="text-sm text-slate-400 mb-6 leading-relaxed">
              Every file passes through two security layers before it is staged. If any check fails,
              the file is <strong className="text-red-400">blocked</strong> and never reaches your remote storage.
            </p>

            <div className="grid sm:grid-cols-2 gap-4 mb-6">
              <div className="glass rounded-xl p-5">
                <h3 className="text-sm font-semibold text-white mb-3 flex items-center gap-2">
                  <span>🚫</span> Blocked filenames
                </h3>
                <div className="flex flex-wrap gap-1.5">
                  {[".env", ".env.*", "*.pem", "*.key", "*.p12", "*.pfx",
                    "id_rsa", "id_dsa", "id_ecdsa", "id_ed25519", ".netrc",
                    "credentials", "secrets.yml", "secrets.json"].map(f => (
                    <span key={f} className="mono text-xs text-red-300 bg-red-400/10 border border-red-400/20 px-2 py-0.5 rounded">{f}</span>
                  ))}
                </div>
              </div>

              <div className="glass rounded-xl p-5">
                <h3 className="text-sm font-semibold text-white mb-3 flex items-center gap-2">
                  <span>🔍</span> Secret patterns detected
                </h3>
                <div className="space-y-1.5">
                  {[
                    "AWS Access Key (AKIA…)",
                    "Generic API Key / Token",
                    "RSA / EC Private Key Header",
                    "Hardcoded passwords",
                    "GitHub tokens (ghp_, ghs_…)",
                    "Slack tokens (xox…)",
                    "Database connection URLs",
                  ].map(p => (
                    <div key={p} className="flex items-center gap-2 text-xs text-slate-400">
                      <span className="text-yellow-400">⚠</span>{p}
                    </div>
                  ))}
                </div>
              </div>
            </div>

            <Terminal label="security in action">{`$ minivcs add .

[BLOCKED] .env — sensitive filename. Use --force to override.
[BLOCKED] config.js — potential secret detected:
  Line 7: [AWS Access Key] AKIAIOSFODNN7EXAMPLE
  Line 12: [Generic Password] password = "MyP@ssw0rd123"
[OK] 18 file(s) staged, 2 blocked`}</Terminal>

            <div className="mt-4 p-4 rounded-xl bg-emerald-400/5 border border-emerald-400/15">
              <p className="text-sm text-emerald-300">
                <strong>Zero-trust by default.</strong> No sensitive file ever leaves your machine silently.
                To override, use <code className="mono">--force</code> — but this is logged prominently.
              </p>
            </div>
          </section>

          <Divider />

          {/* ── Ollama / AI ───────────────────────────────────────────────── */}
          <section id="ai" className="mb-16">
            <SectionLabel>AI Features</SectionLabel>
            <h2 className="text-2xl font-bold text-white mb-2">Local AI + Smart merge resolution</h2>
            <p className="text-sm text-slate-400 mb-6 leading-relaxed">
              Two AI-powered features: one runs entirely on your machine (Ollama), the other
              calls a custom API of your choice for powerful merge conflict resolution.
            </p>

            {/* Ollama */}
            <div className="glass rounded-xl p-6 mb-4">
              <div className="flex items-center gap-3 mb-3">
                <span className="text-2xl">🦙</span>
                <div>
                  <h3 className="text-base font-semibold text-white">Ollama Local Search & Commit Messages</h3>
                  <p className="text-xs text-slate-500">100% local — your code never leaves your machine</p>
                </div>
                <span className="badge badge-purple ml-auto">Private</span>
              </div>
              <p className="text-sm text-slate-400 mb-4">
                Uses a locally running Ollama instance (e.g. TinyLlama) to:
              </p>
              <div className="grid sm:grid-cols-2 gap-3 mb-4">
                <div className="p-3 rounded-lg bg-white/[0.02] border border-white/[0.05]">
                  <p className="text-xs font-semibold text-white mb-1">🔎 Codebase Search</p>
                  <p className="text-xs text-slate-400">Ask natural language questions about your code. The LLM reads your files locally and answers.</p>
                </div>
                <div className="p-3 rounded-lg bg-white/[0.02] border border-white/[0.05]">
                  <p className="text-xs font-semibold text-white mb-1">✍️ Auto Commit Messages</p>
                  <p className="text-xs text-slate-400">Generates context-aware, professional commit messages from your list of staged files.</p>
                </div>
              </div>
              <Terminal>{`# Setup (one-time)
ollama pull tinyllama
minivcs config set ollama_model tinyllama
minivcs config set ollama_url http://localhost:11434

# Search your codebase privately
minivcs search "where is JWT token validated"

# Auto-generate commit message
minivcs commit --ai`}</Terminal>
            </div>

            {/* Merge resolve */}
            <div className="glass rounded-xl p-6">
              <div className="flex items-center gap-3 mb-3">
                <span className="text-2xl">⚔️</span>
                <div>
                  <h3 className="text-base font-semibold text-white">Hybrid Merge Conflict Resolution (AI + SBCR)</h3>
                  <p className="text-xs text-slate-500">Intelligent Meta-Resolver: Semantic LLM + Search-Based Combinatorial Optimization (arXiv:2605.16646)</p>
                </div>
                <span className="badge badge-purple ml-auto">Hybrid AI</span>
              </div>

              <p className="text-sm text-slate-400 mb-4">
                Combines the semantic reasoning of Large Language Models (Merge-Bench standard) with the mathematically robust,
                data-independent guarantees of Search-Based Software Engineering (SBCR). If cloud LLMs experience demand spikes or produce empty outputs,
                the meta-resolver <strong className="text-white">automatically rescues</strong> the resolution. Also runs 100% offline without any API key.
              </p>

              <div className="mb-4 space-y-2">
                <p className="text-xs text-slate-500 uppercase tracking-widest mb-3">Sample multi-paradigm ranking</p>
                <ConfidenceBar pct={95} label="Option 1 (Semantic AI)" />
                <ConfidenceBar pct={88} label="Option 2 (SBCR Search)" />
                <ConfidenceBar pct={70} label="Option 3 (Conservative)" />
              </div>

              <Terminal>{`# Cloud AI Setup (Google Gemini or OpenAI):
minivcs config set ai_api_url https://generativelanguage.googleapis.com/v1beta/openai/chat/completions
minivcs config set ai_model gemini-3.5-flash
minivcs config set ai_api_key AIzaSyYourGeminiApiKey

# Or local Ollama / offline:
# minivcs config set ai_api_url http://localhost:11434/v1/chat/completions
# minivcs config set ai_model tinyllama

# Resolve conflict (works with or without an API key!)
minivcs merge-resolve service.py

# Output:
# ═══ AI Merge Conflict Resolutions ═══
# Option 1 — Confidence: 95%
# Rationale: Semantic synthesis: reconciled concurrent modifications into unified implementation.
# ─────────────────────────────────────
# [resolved code shown here]
#
# Option 2 — Confidence: 88%
# Rationale: Search-based combinatorial optimization (SBCR): maximizes dual-parent similarity (82%)...
# ─────────────────────────────────────
# [search-based combination shown here]
#
# Apply which option? [1-3/n to skip]:`}</Terminal>
            </div>
          </section>

          <Divider />

          {/* ── Versioning Engine ─────────────────────────────────────────── */}
          <section id="design" className="mb-16">
            <SectionLabel>Internals</SectionLabel>
            <h2 className="text-2xl font-bold text-white mb-2">Delta versioning engine</h2>
            <p className="text-sm text-slate-400 mb-6 leading-relaxed">
              Unlike the original full-snapshot model, v2 uses a <strong className="text-white">content-addressed object store</strong> —
              identical to how Git works internally.
            </p>

            <div className="grid sm:grid-cols-3 gap-3 mb-6">
              {[
                { icon: "🔑", title: "Content Addressing", desc: "Every file is hashed with SHA-256. Identical content = one blob. 100 commits with the same package.json = 1 object stored." },
                { icon: "🌳", title: "Tree Objects", desc: "Each commit stores a tree object — a mapping of path → blob hash. Reconstructing any snapshot takes O(n) reads, not copies." },
                { icon: "🔗", title: "Commit Chain", desc: "Commits form a linked list via parent pointers. Log, diff, and checkout walk this chain. Branch pointers are just named refs." },
              ].map(({ icon, title, desc }) => (
                <div key={title} className="glass rounded-xl p-4">
                  <span className="text-xl mb-2 block">{icon}</span>
                  <h3 className="text-sm font-semibold text-white mb-1">{title}</h3>
                  <p className="text-xs text-slate-500">{desc}</p>
                </div>
              ))}
            </div>

            <div className="glass rounded-xl p-5">
              <p className="section-label mb-4">Repository structure</p>
              <Terminal label="filesystem">{`.miniVCS/
├── config            # backend, username, AI keys, Ollama config
├── HEAD              # ref: refs/heads/main
├── INDEX             # staging area (hash → path mappings)
├── objects/          # content-addressed blob store
│   └── ab/
│       └── cdef1234... # raw file content (SHA-256 addressed)
├── commits/          # commit metadata
│   └── commit_20261013_142301/
│       └── meta.txt  # id, tree, parent, author, message
└── refs/
    └── heads/
        ├── main      # → commit_20261013_142301
        └── feature-x # → commit_20261010_091212`}</Terminal>
            </div>
          </section>

          <Divider />

          {/* ── Architecture ─────────────────────────────────────────────── */}
          <section id="architecture" className="mb-16">
            <SectionLabel>Architecture</SectionLabel>
            <h2 className="text-2xl font-bold text-white mb-6">How it fits together</h2>

            {/* Flow */}
            <div className="flex flex-col sm:flex-row items-center gap-2 mb-8">
              {[
                { title: "User", sub: "CLI commands" },
                "→",
                { title: "SecureVCS CLI", sub: "Node.js + C++ binary" },
                "→",
                { title: "Object Store", sub: ".miniVCS/objects" },
                "→",
                { title: "Your Storage", sub: "S3 / GCS / MinIO / …" },
              ].map((node, i) =>
                node === "→" ? (
                  <div key={i} className="text-slate-600 text-xl hidden sm:block">→</div>
                ) : typeof node === "object" ? (
                  <div key={i} className="glass rounded-xl p-4 text-center flex-1 min-w-0">
                    <p className="text-sm font-semibold text-white">{node.title}</p>
                    <p className="text-xs text-slate-500 mt-0.5">{node.sub}</p>
                  </div>
                ) : null
              )}
            </div>

            <div className="space-y-3">
              {[
                { cmd: "add", desc: "Hashes each file, stores blob in content-addressed object store, updates INDEX." },
                { cmd: "commit", desc: "Builds tree object from INDEX, writes commit metadata, advances branch ref, auto-pushes new blobs to remote." },
                { cmd: "diff", desc: "Reads two tree objects, finds changed paths, runs Myers diff on each pair of blobs, renders colored hunks." },
                { cmd: "push / pull", desc: "Syncs the entire objects/, commits/, and refs/ tree with your remote backend using the backend-specific CLI tool." },
                { cmd: "checkout", desc: "Reads tree from commit object, fetches blobs (pulls from remote if missing), reconstructs working directory." },
                { cmd: "merge-resolve", desc: "Reads conflict file, sends to AI API, presents ranked resolutions, applies chosen one." },
              ].map(({ cmd, desc }) => (
                <div key={cmd} className="flex gap-4 text-sm text-slate-400">
                  <span className="mono text-xs bg-indigo-500/10 border border-indigo-500/20 text-indigo-300 rounded px-2 py-0.5 h-fit shrink-0 mt-0.5 whitespace-nowrap">
                    {cmd}
                  </span>
                  <span className="leading-relaxed">{desc}</span>
                </div>
              ))}
            </div>
          </section>

          <Divider />

          {/* ── Build ──────────────────────────────────────────────────────── */}
          <section id="build" className="mb-16">
            <SectionLabel>Build from source</SectionLabel>
            <h2 className="text-2xl font-bold text-white mb-4">Compile the C++ binary</h2>
            <Terminal label="build">{`# With OpenSSL (recommended - hardware SHA-256)
g++ -std=c++17 -O2 main.cpp -lssl -lcrypto -o miniVCS

# Without OpenSSL (uses built-in pure-C++ SHA-256)
g++ -std=c++17 -O2 -DMINIVCS_NO_OPENSSL main.cpp -o miniVCS

# Windows (MinGW / MSYS2)
g++ -std=c++17 -O2 main.cpp -lssl -lcrypto -lws2_32 -lgdi32 -o miniVCS.exe

# macOS (Homebrew OpenSSL)
g++ -std=c++17 -O2 main.cpp -L$(brew --prefix openssl)/lib -I$(brew --prefix openssl)/include -lssl -lcrypto -o miniVCS`}</Terminal>
          </section>

          {/* ── Footer author card ─────────────────────────────────────────── */}
          <div className="glass rounded-xl px-5 py-5 flex items-center gap-4 mb-8">
            <div className="w-10 h-10 rounded-full bg-indigo-600/20 border border-indigo-500/30 flex items-center justify-center shrink-0">
              <span className="mono text-xs font-bold text-indigo-300">GK</span>
            </div>
            <div className="flex-1 min-w-0">
              <p className="text-sm font-semibold text-white">Gulshan Kumar</p>
              <p className="text-xs text-slate-500">Author · SecureVCS · MIT License</p>
            </div>
            <a
              href="https://github.com/GulshanJha00"
              target="_blank"
              rel="noopener noreferrer"
              className="flex items-center gap-1.5 text-xs text-slate-400 hover:text-white border border-white/[0.07] rounded-lg px-3 py-1.5 hover:bg-white/[0.05] transition-all shrink-0"
            >
              <GitHubIcon className="w-3.5 h-3.5" />
              <span className="hidden xs:inline">GitHub</span>
            </a>
          </div>

          {/* Footer */}
          <footer className="border-t border-white/[0.05] py-10">
            <div className="flex flex-col sm:flex-row items-start sm:items-center justify-between gap-4">
              <div>
                <p className="mono text-sm font-semibold text-white mb-1">SecureVCS</p>
                <p className="text-xs text-slate-600">
                  Cloud-native VCS · MIT License · Built by{" "}
                  <a href="https://github.com/GulshanJha00" className="text-slate-500 hover:text-white transition-colors">Gulshan Kumar</a>
                </p>
              </div>
              <div className="flex items-center gap-3">
                <a href="https://www.npmjs.com/package/minivcs" target="_blank" rel="noopener noreferrer"
                  className="flex items-center gap-1.5 text-xs text-slate-500 hover:text-white transition-colors">
                  <NpmIcon className="w-3.5 h-3.5 text-red-400" />npm
                </a>
                <span className="text-slate-800">·</span>
                <a href="https://github.com/GulshanJha00/minivcs" target="_blank" rel="noopener noreferrer"
                  className="flex items-center gap-1.5 text-xs text-slate-500 hover:text-white transition-colors">
                  <GitHubIcon className="w-3.5 h-3.5" />GitHub
                </a>
              </div>
            </div>
          </footer>
        </div>
      </div>
    </>
  );
}