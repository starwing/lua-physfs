local lunit = require "luaunit"
local physfs = require "physfs"

local eq    = lunit.assert_equals
local match = lunit.assert_str_matches
local fail  = lunit.assert_error_msg_matches

print("version: ", physfs.version())
print("types: " .. #assert(physfs.supportedArchiveTypes()))
print("cdRom: " .. #assert(physfs.cdRomDirs()))
print("files: " .. #assert(physfs.files "/"))

function _G.testBaseInfo()
   print("baseDir: " .. assert(physfs.baseDir))

   assert(physfs.mount ".")
   eq(physfs.mountPoint ".", "/")
   eq(#physfs.searchPath(), 1)
   assert(physfs.unmount ".")
   eq(physfs.mountPoint ".", nil)
   eq(#physfs.searchPath(), 0)
   assert(physfs.mount ".")

   local dir = assert(physfs.prefDir("github", "starwing"))
   match(dir, ".-starwing.*")
   assert(physfs.saneConfig("github", "starwing"))
   match(physfs.writeDir(), ".-starwing.*")
   assert(physfs.unmount(dir))

   local useSymlink = physfs.useSymlink()
   physfs.useSymlink(true)
   physfs.useSymlink(useSymlink)

   physfs.lastError "OUT_OF_MEMORY"
   eq(physfs.lastError(), "OUT_OF_MEMORY")
   physfs.lastError "OK"

   assert(physfs.writeDir ".")
   match(physfs.writeDir(), "%.")
end

function _G.testFile()
   local fh = assert(physfs.openWrite "_test_file")
   assert(#fh == 0)
   eq(fh:eof(), false) -- only openRead could return true
   fail("read: file open for writing", assert, fh:read())
   assert(fh:write())
   assert(fh:write("foobar"))
   fail("bad argument #2 to.-%(invalid empty format.*", fh.writeInt, fh, "")
   fail(".-invalid format char 'z'.*", fh.writeInt, fh, "z<z4ziz")
   fail(".-invalid format char 'z'.*", fh.writeInt, fh, "<z4ziz")
   fail(".-invalid format char 'z'.*", fh.writeInt, fh, "<4ziz")
   fail(".-invalid format char 'z'.*", fh.writeInt, fh, "<4iz")
   local fmts = {}
   local piece = {
      l0 = "", l1 = "<", l2 = ">",
      w0 = "", w1 = "1", w2 = "2", w3 = "4", w4 = "8",
      s1 = "i", s2 = "u"
   }
   for little = 0, 2 do
      for wide = 0, 4 do
         for sig = 1, 2 do
            fmts[#fmts+1] = piece["l"..little]..piece["w"..wide]..piece["s"..sig]
         end
      end
   end
   for _, fmt in ipairs(fmts) do
      assert(fh:writeInt(fmt, 1,2,3,4))
   end
   assert(fh:flush())
   eq(fh.__tostring(1), nil)
   match(tostring(fh), "physfs.File: 0x%x+")
   assert(fh:close())
   match(tostring(fh), "physfs.File: %(null%)")

   local stat = assert(physfs.stat "_test_file")
   eq(stat.size, 462)
   eq(stat.type, "file")
   eq(stat.readonly, false)

   fh = assert(physfs.openRead "_test_file")
   assert(fh:buffSize(1024))
   assert(#fh == 462)
   assert(fh:length() == 462)
   eq(fh:eof(), false)
   eq(fh:tell(), 0)
   eq(#assert(fh:read()), 462)
   eq(fh:eof(), true)
   eq(fh:tell(), 462)
   assert(fh:seek(0))
   eq(fh:tell(), 0)
   assert(fh:write())
   fail(".-file open for reading", assert, fh:write "")
   eq(assert(fh:read(6)), "foobar")
   assert(fh:seek(0))
   eq(assert(fh:read(6)), "foobar")
   assert(fh:tell(), 6)
   for _, fmt in ipairs(fmts) do
      local a, b, c, d, e = fh:read(fmt, fmt, fmt, fmt)
      eq(a, 1); eq(b, 2); eq(c, 3); eq(d, 4); eq(e, nil)
   end
   assert(fh:close())
   assert(physfs.exists "_test_file")
   eq(physfs.realDir "_test_file", ".")
   assert(physfs.delete "_test_file")
   assert(not physfs.exists "_test_file")
end

function _G.testDir()
   assert(physfs.mkdir "_test_dir")
   eq(physfs.stat "_test_dir".type, "dir")
   assert(physfs.mkdir "_test_dir/a")
   assert(physfs.mkdir "_test_dir/b")
   assert(physfs.mkdir "_test_dir/c")
   assert(physfs.delete "_test_dir/c")
   assert(physfs.delete "_test_dir/b")
   assert(physfs.delete "_test_dir/a")
   assert(physfs.delete "_test_dir")
end

function _G.testConv()
   eq(physfs.convInt("<1i", 4), 4)
   eq(physfs.convInt("<2i", 4), 4)
   eq(physfs.convInt("<4i", 4), 4)
   eq(physfs.convInt("<8i", 4), 4)
   eq(physfs.convInt(">1i", 4), 4)
   eq(physfs.convInt(">2i", 4), 0x0400)
   eq(physfs.convInt(">4i", 4), 0x04000000)
   eq(physfs.convInt(">8i", 4), 0x0400000000000000)
   eq(physfs.convInt("<1u", 4), 4)
   eq(physfs.convInt("<2u", 4), 4)
   eq(physfs.convInt("<4u", 4), 4)
   eq(physfs.convInt("<8u", 4), 4)
   eq(physfs.convInt(">1u", 4), 4)
   eq(physfs.convInt(">2u", 4), 0x0400)
   eq(physfs.convInt(">4u", 4), 0x04000000)
   eq(physfs.convInt(">8u", 4), 0x0400000000000000)
end

function _G.testLoader()
   fail(".-physfs search path.*", require, "foobar")

   local function test_mod(m)
      eq(m.info(), "this is a test module from physfs")
      eq(m.add(1,2), 3)
   end
   assert(physfs.mount "test_mod.zip")
   test_mod(require "test_mod")

   local fh = assert(physfs.openRead "test_mod.zip")
   match(tostring(fh), "physfs.File: 0x%x+")
   local fh2 = assert(physfs.mountFile(fh, nil, "test2"))
   eq(fh, fh2)
   match(tostring(fh), "physfs.File: %(null%)")
   -- NOTICE there is a bug, if not given name to mount(File|Memory),
   -- physfs libary has several bugs about exists and searchPath.
   -- eq(physfs.exists("test2/test_mod.lua"), true) -- fail
   test_mod(require "test2.test_mod")

   fh = assert(physfs.openRead "test_mod.zip")
   local content = assert(fh:read())
   eq(#content, #fh)
   assert(fh:close())
   assert(physfs.mountMemory(content, nil, "test3"))
   test_mod(require "test3.test_mod")
end

os.exit(lunit.LuaUnit.run(), true)

