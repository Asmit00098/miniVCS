#!/usr/bin/env node

const { spawn } = require("child_process");
const path = require("path");

const args = process.argv.slice(2);

let executable;

const fs = require("fs");

if (process.platform === "win32") {
    // Windows: check bin/miniVCS.exe or bin/windows/miniVCS.exe
    const candidates = [
        path.join(__dirname, "bin", "miniVCS.exe"),
        path.join(__dirname, "bin", "windows", "miniVCS.exe"),
        path.join(__dirname, "bin", "windows", "minivcs.exe"),
        path.join(__dirname, "miniVCS.exe")
    ];
    executable = candidates.find(p => fs.existsSync(p)) || candidates[0];
} else if (process.platform === "darwin") {
    // macOS
    const candidates = [
        path.join(__dirname, "bin", "miniVCS"),
        path.join(__dirname, "bin", "mac", "miniVCS"),
        path.join(__dirname, "miniVCS")
    ];
    executable = candidates.find(p => fs.existsSync(p)) || candidates[0];
} else {
    // Linux
    const candidates = [
        path.join(__dirname, "bin", "linux", "miniVCS"),
        path.join(__dirname, "bin", "miniVCS"),
        path.join(__dirname, "miniVCS")
    ];
    executable = candidates.find(p => fs.existsSync(p)) || candidates[0];
}

const proc = spawn(executable, args, {
    stdio: "inherit"
});

proc.on("error", (err) => {
    console.error("Failed to start miniVCS:", err.message);
});

proc.on("close", (code) => {
    if (code !== 0) {
        console.log("\nminiVCS command failed.");
    }

    process.exit(code ?? 1);
});