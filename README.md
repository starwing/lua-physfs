physfs binding for Lua
----------------------
[![Build Status](https://img.shields.io/github/actions/workflow/status/starwing/lua-physfs/test.yml?branch=master)](https://github.com/starwing/lua-physfs/actions?query=branch%3Amaster)[![Coverage Status](https://img.shields.io/coveralls/github/starwing/lua-physfs)](https://coveralls.io/github/starwing/lua-physfs?branch=master)

lua-physfs is a [physfs][1] binding for the Lua language. it expose
most physfs API, except the `mountIo()` and `register()` routines.

PhysicsFS is a library to provide abstract access to various archives. 

to use this binding, easiest way is use [luarocks][2]:

```
luarocks install physfs
```

this library is in Lua license, same as the Lua language.

API List
-------------

- physfs.File:
  - `#file                         -> number|error`
  - `file:buffSize(number)         -> file|(nil, errmsg)`
  - `file:close()                  -> file|(nil, errmsg)`
  - `file:eof()                    -> boolean`
  - `file:flush()                  -> file|(nil, errmsg)`
  - `file:length()                 -> number|(nil, errmsg)`
  - `file:read(fmt...)             -> (nil|number|string)...`
  - `file:seek(number)             -> file|(nil, errmsg)`
  - `file:tell()                   -> number|(nil, errmsg)`
  - `file:write(string...)         -> file|(nil, errmsg)`
  - `file:writeInt(fmt, number...) -> file|(nil, errmsg)`
  - `tostring(file)                -> string`

- `physfs.cdRomDirs([table])        -> table, number`
- `physfs.convInt(fmt, number...)   -> number...`
- `physfs.delete(string)            -> string|(nil, errmsg)`
- `physfs.exists(string)            -> boolean`
- `physfs.files(string[, table])    -> table, number`
- `physfs.lastError()               -> string`
- `physfs.lastError(string)         -> none`
- `physfs.mkdir(string)             -> string|(nil, errmsg)`
- `physfs.mount(name[, point[, preppend]]) -> name|(nil, errmsg)`
- `physfs.mountFile(file[, name[, point[, preppend]]]) -> file|(nil, errmsg)`
- `physfs.mountMemory(string[, name[, point[, preppend]]]) -> string|(nil, errmsg)`
- `physfs.mountPoint(string)        -> string`
- `physfs.openAppend(string)        -> file|(nil, errmsg)`
- `physfs.openRead(string)          -> file|(nil, errmsg)`
- `physfs.openWrite(string)         -> file|(nil, errmsg)`
- `physfs.prefDir(org, app)         -> org|(nil, errmsg)`
- `physfs.realDir(string)           -> string`
- `physfs.saneConfig(org, app[, ext[, includeCdRoms[, archiveFirst]]]) -> org|(nil, errmsg)`
- `physfs.searchPath([table])       -> table, number`
- `physfs.stat(string[, table])     -> table`
- `physfs.supportedArchiveTypes([table]) -> table, number`
- `physfs.unmount(string)           -> string|(nil, errmsg)`
- `physfs.useSymlink()              -> boolean`
- `physfs.useSymlink(boolean)       -> none`
- `physfs.version()                 -> number, number, number`
- `physfs.writeDir()                -> string`
- `physfs.writeDir(string)          -> string|(nil, errmsg)`

