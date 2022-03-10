const { argv } = require('yargs')(process.argv);

const type = argv.type || 'debug';
const cmake = `cmake${process.platform == 'win32' ? '.exe' : ''}`;

await $`${cmake} --preset win-x64-${type} .`;
await $`${cmake} --build --preset win-x64-${type}`;

await fs.copyFile(
  `./dist/win-x64-${type}/flatt/flatt.exe`,
  `./bin/win-x64/flatt.exe`
);
