luarocks_enable()

local fs = require("flatt.fs")
local strings = require("flatt.strings")
local log = require("flatt.logger")
local templates = require("flatt.templates")
local flatbuffers = require("flatbuffers")

-- compile fbs
local generated = flatbuffers.compile({
  "--gen-all",
  "--gen-onefile",
  "--gen-mutable",
  "--gen-compare",
  "--gen-nullable",
  "--gen-object-api",
  "--gen-name-strings",
  "--scoped-enums",
  "--natural-utf8",
  "--strict-json",
  "--defaults-json",
  "--force-empty",
  "--force-empty-vectors",
  "--reflect-types",
  "--reflect-names",
  "--cpp",
  "--cpp-include", "array",
  "--cpp-std", "c++17",
  "--cpp-field-case-style", "lower",
  "--cpp-static-reflection",
  "--filename-suffix", "",
  "-o", "./src",
  "./ecs.fbs",
})

if not generated then
  log.error("Failed to generate files with flatc")
  return 1
end

-- type information
local reflection = flatbuffers.reflect("./ecs.fbs")
if not reflection then
  log.error("Failed to generate type information...")
  return 1
end

fs.write_json("ecs.json", reflection)

-- manipulate reflection data and render templates
local components = {}
for _, current in pairs(reflection.tables) do
  if current.attributes.component then
    current.attributes.is_toggleable = current.attributes.is_toggleable or false
    current.attributes.is_singleton = current.attributes.is_singleton or false
    current.attributes.is_sparse = current.attributes.is_sparse or false
    current.name_snake = strings.snake(current.name)
    current.namespace_path = strings.cpp(current.namespace)
    table.insert(components, current)
  end
end

local enums = {}
for _,current in pairs(reflection.enums) do
  table.insert(enums, current)
end

-- generate code

local data = {
  components = components,
  enums = enums,
}

local files = fs.list_files("./template")
for _, file in pairs(files) do
  local source = './template/' .. file
  local target = "./src/" .. file
  local input = fs.read_file(source)
  local output = input

  if strings.ends_with(source, ".j2") then
    target = target:sub(0, #target - 3)
    output = templates.render(input, data)
  end

  log.warn("writing file to " .. target)
  fs.write_file(target, output)
end

-- finish

log.info("Done.")
