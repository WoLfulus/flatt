local logger = require("flatt.logger")
logger.set_level("trace")

luarocks_enable()

local json = luarocks_require("lunajson")
local array = luarocks_require("array")
local fun = luarocks_require("fun")

local function hey(name: string)
  print("Hey! I can load " .. name .. " files")
end

local data = json.decode('{ "name": "Teal Language" }')
hey(data.name)
