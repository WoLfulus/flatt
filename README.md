# flatt

Flatbuffers ~code~ reflection based generation tool with scripting support.

# Why?

Flatbuffers is awesome, but you'll eventually need to (or wish you could) write additional code generation steps for your schema files. For example, generating a network protocol handler/dispatcher for you client and server and keep them in sync.

# How does it work?

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

# Usage

## Installation

- `npm install -g flatt`

## Requirements

- Windows (PRs welcome to add Linux+WSL and Mac support)

## Overview

> `flatt some/project.lua`

##

# Building

## Requirements

- Windows
- Visual Studio 2022
- CMake
- Conan
- Node.js

## IDE (Windows)

- `scoop install cmake`
- `scoop install llvm`

## Compiling

> `npm install && npm run build`

# API

## `vars`

---

### `flatt.vars.flatt_dir`

Has the `flatt` executable directory.

### `flatt.vars.project_dir`

Project directory (from the given project script file).

### `flatt.vars.argv`

A table (list) of arguments passed next to the script file.

```
$ flatt project.lua --hello --world abc 123
```

```lua
flatt.vars.argv
-- { "--hello", "--world", "abc", "123" }

flatt.vars.argv[1]
-- { "--hello" }
```

```lua
-- `luarocks install argparse`
local argparse = require("argparse")

local parser = argparse("example", "Arguments example.")
parser:flag("-h --hello", "Hello argument.", "")
parser:flag("-w --world", "World argument.", "")
parser:argument("a", "Some value")
parser:argument("b", "Some value")

local args = parser:parse(flatt.vars.argv)

args.hello
-- true

args.world
-- false

args.a
-- "abc"

args.b
-- "123"
```

---

## `flatc`

---

### `flatt.flatc(arguments)`

```lua
flatt.flatc({ "--cpp", "schema.fbs" })
-- return: the exit code
```

---

## `shell`

---

### `flatt.shell(command, arguments)`

```lua
flatt.shell("echo", { "hello", "world" })
-- return: the exit code
```

---

## `reflect`

---

### `flatt.reflect(path)`

```lua
flatt.reflect("schema.fbs")
--[[
  {
    // JSON encoded schema
  }
]]
```

---

## `log`

---

<details>
  <summary>
    Functions
  </summary>

### `Level` list

- `trace`
- `debug`
- `info`
- `warn`
- `error`
- `critical`

### `flatt.file.set_level(level)`

```lua
flatt.log.set_level("info")
-- Sets the current logging level
```

### `flatt.file.get_level()`

```lua
flatt.log.get_level()
-- Gets the current logging level
```

### `flatt.file.trace(line)`

```lua
flatt.log.trace("log line")
-- Writes to console in white (windows)
```

### `flatt.file.trace(line)`

```lua
flatt.log.trace("log line")
-- Writes to console in white (windows)
```

### `flatt.file.debug(line)`

```lua
flatt.log.debug("log line")
-- Writes to console in blue (windows)
```

### `flatt.file.info(line)`

```lua
flatt.log.info("log line")
-- Writes to console in green (windows)
```

### `flatt.file.warn(line)`

```lua
flatt.log.warn("log line")
-- Writes to console in yellow (windows)
```

### `flatt.file.error(line)`

```lua
flatt.log.error("log line")
-- Writes to console in red (windows)
```

### `flatt.file.critical(line)`

```lua
flatt.log.critical("log line")
-- Writes to console in white with red background (windows)
```

</details>

---

## `file`

---

<details>
  <summary>
    Functions
  </summary>

### `flatt.file.write(path, data)`

```lua
flatt.dir.read("hello.txt", "Hello world!)
-- return: "Hello world!"
```

### `flatt.file.read(path)`

```lua
flatt.dir.read("hello.txt")
-- return: "Hello world!"
```

### `flatt.file.exists(path)`

```lua
flatt.dir.exists("./non-existing-file.ext")
-- return: false

flatt.dir.exists("hello.txt")
-- return: true
```

### `flatt.file.hash(path)`

```lua
flatt.dir.hash("./non-existing-file.ext")
-- return: nil

flatt.dir.hash("some-file.ext")
-- return: "EDCBA5DD333A60F3C98452672A1AB1711409040D"
```

</details>

---

## `dir`

---

<details>
  <summary>
    Functions
  </summary>

### `flatt.dir.hash(path)`

```lua
flatt.dir.hash("./non-existing-dir")
-- return: nil

flatt.dir.hash(".")
-- return: "4CF8ADEDB3B43E78645E4DE673D2D7DD4CFADA58"
```

</details>

---

## `templates`

---

Builtin provided template engine functions (inja).
Use `aspect` for a more Lua friendly template system.

<details>
  <summary>
    Functions
  </summary>

### `flatt.templates.render_string(template, data)`

> Accepts JSON encoded string for data.

```lua
flatt.templates.render_string("hello {{name}}", "{\"name\":\"world\"}")
-- return: "hello world"
```

### `flatt.templates.render_file(src, dest, data)`

> Accepts JSON encoded string for data.

```lua
flatt.templates.render_string("template.txt", "output.txt", "{\"name\":\"world\"}")
-- return: true
```

---

## `string`

---

<details>
  <summary>
    Functions
  </summary>

### `flatt.string.pad_left(string, len, pad = " ")`

```lua
flatt.string.pad_left("1234", 8)
-- return: "    1234"

flatt.string.pad_left("1234", 8, "0")
-- return: "00001234"
```

### `flatt.string.pad_right(string, len, pad = " ")`

```lua
flatt.string.pad_left("1234", 8)
-- return: "1234    "

flatt.string.pad_right("1234", 8, "0")
-- return: "12340000"
```

### `flatt.string.starts_with(string, string)`

```lua
flatt.string.starts_with("hello world", "hello")
-- return: true

flatt.string.starts_with("hello world", "world")
-- return: false
```

### `flatt.string.ends_with(string, string)`

```lua
flatt.string.ends_with("hello world", "hello")
-- return: false

flatt.string.ends_with("hello world", "world")
-- return: true
```

### `flatt.string.split(string, delim, limit = 0)`

```lua
flatt.string.split("hello world", " ")
-- return: {"hello", "world"}

flatt.string.split("is this real life", " ", 3)
-- return: {"is", "this", "real life"}
```

### `flatt.string.trim(string)`

```lua
flatt.string.trim("  hello    world  ")
-- return: "hello    world"
```

### `flatt.string.trim_left(string)`

```lua
flatt.string.trim("  hello    world  ")
-- return: "hello    world  "
```

### `flatt.string.trim_right(string)`

```lua
flatt.string.trim("  hello    world")
-- return: "hello    world  "
```

### `flatt.string.explode(string)`

> Splits the string whenever one of the following characters is found: `. -_|/\`

```lua
flatt.string.explode("a.b c-d|e/f\\g")
-- return: { "a", "b", "c", "d", "e", "f", "g" }
```

### `flatt.string.join(strings, delim = ",")`

```lua
flatt.string.join({ "a", "b", "c", "d", "e", "f", "g" }, "~")
-- return: "a~b~c~d~e~f~g"
```

### `flatt.string.lower_case(string)`

```lua
flatt.string.lower_case("NewGUIAlertBox")
-- return: "newguialertbox"
```

### `flatt.string.upper_case(string)`

```lua
flatt.string.upper_case("NewGUIAlertBox")
-- return: "NEWGUIALERTBOX"
```

### `flatt.string.ucfirst(string)`

```lua
flatt.string.ucfirst("NewGUIAlertBox")
-- return: "NewGUIAlertBox"
```

### `flatt.string.lcfirst(string)`

```lua
flatt.string.lcfirst("NewGUIAlertBox")
-- return: "newGUIAlertBox"
```

### `flatt.string.snake_case(string)`

```lua
flatt.string.snake_case("NewGUIAlertBox")
-- return: "new_gui_alert_box"
```

### `flatt.string.kebab_case(string)`

```lua
flatt.string.kebab_case("NewGUIAlertBox")
-- return: "new-gui-alert-box"
```

### `flatt.string.pascal_case(string)`

```lua
flatt.string.pascal_case("NewGUIAlertBox")
-- return: "NewGuiAlertBox"
```

### `flatt.string.camel_case(string)`

```lua
flatt.string.camel_case("NewGUIAlertBox")
-- return: "newGuiAlertBox"
```

### `flatt.string.const_case(string)`

```lua
flatt.string.const_case("NewGUIAlertBox")
-- return: "NEW_GUI_ALERT_BOX"
```

### `flatt.string.train_case(string)`

```lua
flatt.string.train_case("NewGUIAlertBox")
-- return: "New-Gui-Alert-Box"
```

### `flatt.string.ada_case(string)`

```lua
flatt.string.ada_case("NewGUIAlertBox")
-- return: "New_Gui_Alert_Box"
```

### `flatt.string.cobol_case(string)`

```lua
flatt.string.cobol_case("NewGUIAlertBox")
-- return: "NEW-GUI-ALERT-BOX"
```

### `flatt.string.dot_case(string)`

```lua
flatt.string.dot_case("NewGUIAlertBox")
-- return: "new.gui.alert.box"
```

### `flatt.string.path_case(string)`

```lua
flatt.string.path_case("NewGUIAlertBox")
-- return: "new/gui/alert/box"
```

### `flatt.string.space_case(string)`

```lua
flatt.string.space_case("NewGUIAlertBox")
-- return: "new gui alert box"
```

### `flatt.string.capital_case(string)`

```lua
flatt.string.capital_case("NewGUIAlertBox")
-- return: "New Gui Alert Box"
```

### `flatt.string.cpp_case(string)`

```lua
flatt.string.cpp_case("NewGUIAlertBox")
-- return: "new::gui::alert::box"
```

</details>
