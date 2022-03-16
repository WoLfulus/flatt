#!/usr/bin/env node

const fs = require('fs');
const os = require('os');
const path = require('path');
const here = path.dirname(__filename);

const platform = {
  id: `${process.platform.replace(/\d*/g, '')}-${os.arch()}`,
  suffix: process.platform === 'win32' ? '.exe' : '',
};

const flatc = `${here}/${platform.id}/flatc${platform.suffix}`;
const flatt = `${here}/${platform.id}/flatt${platform.suffix}`;

if (!fs.existsSync(flatt) || !fs.existsSync(flatc)) {
  console.log(`Sorry, flatt doesn't seem to have a binary available for platform ${platform.id}`);
  process.exit(1);
} else {
  async function main() {
    const args = process.argv.splice(2);
    const { execa } = await import('execa');
    const proc = execa(flatt, args, { stderr: 'inherit', stdout: 'inherit', stdin: 'inherit' });
    try {
      console.log(await proc);
    } catch (err) {
      process.exit(1);
    }
  };
  main();
}
