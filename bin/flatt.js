#!/usr/bin/env node

const fs = require("fs");
const os = require("os");
const path = require("path");
const here = path.dirname(__filename);

const platform = `${process.platform.replace(/\d*/g, "")}-${os.arch()}-release`;
const suffix = process.platform === "win32" ? ".exe" : "";

const flatc = `${here}/${platform}/flatc${suffix}`;
const flatt = `${here}/${platform}/flatt${suffix}`;

if (!fs.existsSync(flatt) || !fs.existsSync(flatc)) {
  console.log(
    `Sorry, flatt doesn't seem to have a binary available for platform ${platform}`
  );
  process.exit(1);
} else {
  async function main() {
    const args = process.argv.splice(2);
    const { execa } = await import("execa");
    const proc = execa(flatt, args, {
      stderr: "inherit",
      stdout: "inherit",
      stdin: "inherit",
    });
    try {
      await proc;
    } catch (err) {
      process.exit(1);
    }
  }
  main();
}
