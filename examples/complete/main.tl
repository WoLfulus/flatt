
luarocks_enable()

local json = rock("lunajson")
local array = rock("array")
local fun = rock("fun")

print(json)
print(array)
print(fun)

local function hey(name: string)
  print("Hey! I can load " .. name .. " files")
end

local data = json.decode('{ "name": "Teal Language" }')
hey(data.name)
