-- imports

local json = require("lunajson")

-- compile fbs
local generated = fb.compile({
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
local reflection_json = fb.reflect("./ecs.fbs")
if reflection_json == "" then
  log.error("Failed to generate type information...")
  return 1
end

local reflection = json.decode(reflection_json)

-- dump to local json for debugging
file.write("ecs.json", reflection_json)

-- manipulate reflection data and render templates
local components = {}
for _, current in pairs(reflection.tables) do
  if current.attributes.component then
    current.attributes.is_toggleable = current.attributes.is_toggleable or false
    current.attributes.is_singleton = current.attributes.is_singleton or false
    current.attributes.is_sparse = current.attributes.is_sparse or false
    current.name_snake = string.snake_case(current.name)
    current.namespace_path = string.cpp_case(current.namespace)
    table.insert(components, current)
  end
end

local enums = {}
for _,current in pairs(reflection.enums) do
  table.insert(enums, current)
end

-- generate code

local template_data = json.encode({
  components = components,
  enums = enums,
})

local files = dir.list_files("./template")
for _,tpl in pairs(files) do
  local source = './template/' .. tpl
  local destination = "./src/" .. tpl

  if string.ends_with(source, ".j2") then
    -- Renders the template
    destination = destination:sub(0, #destination - 3)
    file.write(destination, template.render_string(file.read(source), template_data))
  else
    -- Copy the file
    file.write(destination, file.read(source))
  end
end

-- finish

log.info("Done.")
