import { expandGlobSync } from "jsr:@std/fs";
import { relative, resolve } from "jsr:@std/path";

const root = resolve("./src/modules");

const modules = Array.from(
  expandGlobSync("**/*.{lua}", {
    root,
    extended: true,
    globstar: true,
    canonicalize: true,
  })
).flatMap((file) => {
  const name = relative(root, file.path)
    .replace(/.lua$/, "")
    .replaceAll("/", ".")
    .replaceAll("\\", ".");
  const modules = [
    {
      name,
      file: file.path,
    },
  ];
  return modules;
});

let sorted_modules = modules.sort((a, b) => a.name.localeCompare(b.name));

let dump = "";
let offset = 0;

const module_positions = [] as Array<{
  name: string;
  start: number;
  end: number;
}>;

for (const mod of sorted_modules) {
  const data = readFile(mod.file);
  dump += `  // module: ${mod.name}\n`;
  dump += `  // length: ${data.length}\n`;
  dump += `  //  start: ${offset}\n`;
  dump += `  //    end: ${offset + data.length}\n`;
  dump += data.contents + "\n";
  module_positions.push({
    name: mod.name,
    start: offset,
    end: offset + data.length,
  });

  if (mod.name.endsWith(".init")) {
    module_positions.push({
      name: mod.name.replace(/\.init$/, ""),
      start: offset,
      end: offset + data.length,
    });
  }
  offset += data.length;
}

const header = `#pragma once

#include <sol/sol.hpp>

void register_embedded_modules(sol::state& lua);
`;

let cpp = `#include "modules.hpp"

const unsigned char embedded_modules[] = {
${dump}
};

int embedded_loader(lua_State* L) {
	std::string path = sol::stack::get<std::string>(L, 1);

  ${module_positions
    .map(
      (mod) => `
  if (path == "${mod.name}") {
    auto script = std::string(&embedded_modules[${mod.start}], &embedded_modules[${mod.end}]);
    luaL_loadbuffer(L, script.data(), script.size(), path.c_str());
    return 1;
  }`
    )
    .join("\n  ")}

	sol::stack::push(L, "This is not the module you're looking for!");
	return 1;
}


void register_embedded_modules(sol::state& lua) {

  lua.safe_script(R"(
    function __has_module__(name)
      local __p__, __m__ = pcall(require, name);
      if __p__ then
        return true
      else
        return false
      end
    end
  )");

  sol::function module_exists = lua["__has_module__"];

  lua.add_package_loader(embedded_loader);

}

`;

Deno.writeFileSync("src/modules.hpp", new TextEncoder().encode(header), {
  create: true,
});

Deno.writeFileSync("src/modules.cpp", new TextEncoder().encode(cpp), {
  create: true,
});

function readFile(file: string) {
  const buffer = Deno.readFileSync(file);

  const lines = [];

  const chunkSize = 16;
  for (let i = 0; i < buffer.length; i += chunkSize) {
    const line = Array.from(buffer.subarray(i, i + chunkSize)).map(
      (byte) => `0x${byte.toString(16).padStart(2, "0")}`
    );
    lines.push(`  ${line.join(", ")},`);
  }

  return {
    length: buffer.length,
    contents: lines.join("\n"),
  };
}
