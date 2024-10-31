# flatt

Flatbuffers ~code~ reflection based generation tool with scripting support.

## Why?

Flatbuffers is awesome, but you'll eventually need to (or wish you could) write additional code generation steps for your schema files. For example, generating a network protocol handler/dispatcher for you client and server and keep them in sync.

## How does it work?

`flatt` exposes a `lua` runtime with some additional helper functions to help you deal with Flatbuffers schema files, reflection and template rendering. You can for example write a `flatt` script that will reflect your schema, manipulate that data, and render (using a template engine or not) a file with the content you just processed.

All builtin lua libraries are exported and you can use `luarocks` packages with it if you want to.

> An example of what a flatt script looks like:

```lua
-- luarocks install lunajson
local lunajson = require("lunajson")

flatt.log.info("Generating headers...")
flatt.flatc({ "--cpp", "./schema.fbs" })

local json = flatt.reflect("./schema.fbs")
flatt.log.trace(json)

--[[
  {
    "tables": [
      {
        "id": 182975129,
        "name": "MyTable",
        "namespace": "my.namespace",
        "fields": [
          {
            "name": "some_field",
            ...
          }
        ]
      }
      ...
    ],
    "structs": [
      ...
    ],
    ...
  }
]]

local info = lunajson.decode(json)

-- generate a file using `info` data

```

## Usage

### Installation

- `npm install -g flatt`

### Requirements

- Windows (PRs welcome to add Linux+WSL and Mac support)

### Overview

> `flatt some/project.lua`

###

## Building

### Requirements

- `scoop install cmake`
- `scoop install charm-gum`
- `scoop install just`

### Compiling

> `npm install && npm run build`

---

# API Docs

> WIP
