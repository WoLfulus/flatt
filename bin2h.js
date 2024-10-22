// TODO: file from argv

const fs = require("fs");
const buffer = fs.readFileSync(__dirname + "/tl.lua");

const lines = [];

const chunkSize = 16;
for (let i = 0; i < buffer.length; i += chunkSize) {
  const line = Array.from(buffer.subarray(i, i + chunkSize)).map(
    (byte) => `0x${byte.toString(16).padStart(2, "0")}`
  );

  console.log(line.toString());
  lines.push(`  ${line.join(", ")},`);
}

const output = `
const size_t tl_lua_size = ${buffer.length};

const unsigned char tl_lua[] = {
${lines.join("\n")}
};

const std::string tl_lua_str = std::string(tl_lua, tl_lua + tl_lua_size);
`;

fs.writeFileSync(__dirname + "/tl.lua.h", output);
