-- imports

local array = require("array")
local lunajson = require("lunajson")
local inspect = require("inspect")

-- cli options

flatt.log.set_level("info")

-- generate headers

flatt.log.info("Generating code with flatc...")

local generated = flatt.flatc({ "--cpp", "-o", "./generated/include", "./schema/packets.fbs" })
if not generated then
  flatt.log.error("Failed to generate files with flatc")
  return 1
end

-- type information

flatt.log.info("Fetching type information...")

local reflection = flatt.reflect("./schema/packets.fbs")
if reflection == "" then
  flatt.log.error("Failed to generate type information...")
  return 1
end

-- decode type information into lua objects

local reflection = lunajson.decode(reflection)

-- generate a version string for the given schema

local version = flatt.dir.hash("./schema")

-- manipulate reflection data and render templates

local packets = {}
for index,current in pairs(reflection.tables) do
  -- use packet attribute as a selector
  if current.attributes.packet then
    current.name_snake = flatt.string.snake_case(current.name)
    current.namespace_path = flatt.string.cpp_case(current.namespace)
    table.insert(packets, current)
  end
end

flatt.log.info("")

-- copy files that are not a template file

local template_files = {}
local files = flatt.dir.list_files("./template")
for i,file in pairs(files) do
  local source = './template/' .. file
  local destination = "./generated/" .. file
  local data = lunajson.encode({
    version = version,
    packets = packets,
  })

  if not flatt.string.ends_with(source, ".j2") then
    flatt.log.info("  Copying: " .. source)
    flatt.file.write(destination, flatt.file.read(source))
  else
    destination = destination:sub(0, #destination - 3) -- remove extension
    flatt.log.info("Rendering: " .. source)
    flatt.file.write(destination, flatt.template.render_string(flatt.file.read(source), data))
  end
end

flatt.log.info("")

