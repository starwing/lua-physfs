#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

#define PHYSFS_STATIC
#include "physfs/src/physfs.h"

#include <stdlib.h>
#include <string.h>

#define error_codes(X)                        \
    X(OK)                 X(OTHER_ERROR)      \
    X(OUT_OF_MEMORY)      X(NOT_INITIALIZED)  \
    X(IS_INITIALIZED)     X(ARGV0_IS_NULL)    \
    X(UNSUPPORTED)        X(PAST_EOF)         \
    X(FILES_STILL_OPEN)   X(INVALID_ARGUMENT) \
    X(NOT_MOUNTED)        X(NOT_FOUND)        \
    X(SYMLINK_FORBIDDEN)  X(NO_WRITE_DIR)     \
    X(OPEN_FOR_READING)   X(OPEN_FOR_WRITING) \
    X(NOT_A_FILE)         X(READ_ONLY)        \
    X(CORRUPT)            X(SYMLINK_LOOP)     \
    X(IO)                 X(PERMISSION)       \
    X(NO_SPACE)           X(BAD_FILENAME)     \
    X(BUSY)               X(DIR_NOT_EMPTY)    \
    X(OS_ERROR)           X(DUPLICATE)        \
    X(BAD_PASSWORD)       X(APP_CALLBACK)


/* utils */

#if LUA_VERSION_NUM < 502
# define LUA_OK                    0
# define lua_rawlen                lua_objlen
# define luaL_setfuncs(L,libs,nup) luaL_register(L, NULL, libs)
 
#ifndef LUA_GCISRUNNING /* not LuaJIT 2.1 */

#define luaL_newlibtable(L,l)      \
    lua_createtable(L, 0, sizeof(l)/sizeof((l)[0]) - 1)
#define luaL_newlib(L,l)           \
  (luaL_newlibtable(L,l), luaL_setfuncs(L,l,0))

static void *luaL_testudata(lua_State *L, int idx, const char *tname) {
    void *p = lua_touserdata(L, idx);
    if (p != NULL) {
        if (lua_getmetatable(L, idx)) {
            luaL_getmetatable(L, tname);
            if (!lua_rawequal(L, -1, -2)) p = NULL;
            lua_pop(L, 2);
            return p;
        }
    }
    return NULL;
}

static void luaL_setmetatable(lua_State *L, const char *tname) {
    luaL_getmetatable(L,tname);
    lua_setmetatable(L,-2);
}

static lua_Integer lua_tointegerx(lua_State *L, int idx, int *isint) {
    lua_Integer v = lua_tointeger(L, idx);
    if (isint) *isint = v != 0 || lua_type(L, idx) == LUA_TNUMBER;
    return v;
}

#endif /* not LuaJIT 2.1 */

#endif

#if LUA_VERSION_NUM < 503
static int lua53_getglobal(lua_State *L, const char *name)
{ lua_getglobal(L, name); return lua_type(L, -1); }
static int lua53_getfield(lua_State *L, int idx, const char *name)
{ lua_getfield(L, idx, name); return lua_type(L, -1); }
#else
# define lua53_getglobal lua_getglobal
# define lua53_getfield  lua_getfield
#endif

#define return_self(L) do { lua_settop(L, 1); return 1; } while (0)

#define api(name, call) do { \
    if (!PHYSFS_##call) return push_error(L, name); } while (0)

static int push_error(lua_State *L, const char *fn) {
    const char *err = PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
    lua_pushnil(L);
    lua_pushfstring(L, "%s: %s", fn, err ? err : "unknown error");
    return 2;
}


/* physfs.File */

#define LFS_FILE "physfs.File"

static PHYSFS_File *check_file(lua_State *L, int idx);

static int Lfile_eof(lua_State *L)
{ lua_pushboolean(L, PHYSFS_eof(check_file(L, 1))); return 1; }

static int Lfile_flush(lua_State *L)
{ api("flush", flush(check_file(L, 1))); return_self(L); }

static PHYSFS_File *check_file(lua_State *L, int idx) {
    PHYSFS_File **pf = (PHYSFS_File**)luaL_checkudata(L, idx, LFS_FILE);
    luaL_argcheck(L, idx, *pf != NULL, "invalid " LFS_FILE " got");
    return *pf;
}

static int Lfile_close(lua_State *L) {
    PHYSFS_File **pf = (PHYSFS_File**)luaL_testudata(L, 1, LFS_FILE);
    if (pf && *pf) {
        api("close", close(*pf));
        *pf = NULL;
    }
    return_self(L);
}

static int Lfile_tostring(lua_State *L) {
    PHYSFS_File **pf = (PHYSFS_File**)luaL_testudata(L, 1, LFS_FILE);
    if (pf == NULL)
        return 0;
    else if (*pf == NULL)
        lua_pushstring(L, LFS_FILE ": (null)");
    else
        lua_pushfstring(L, LFS_FILE ": %p", *pf);
    return 1;
}

static int Lfile_tell(lua_State *L) {
    PHYSFS_File *file = check_file(L, 1);
    PHYSFS_sint64 pos = PHYSFS_tell(file);
    if (pos < 0) return push_error(L, "tell");
    lua_pushinteger(L, (lua_Integer)pos);
    return 1;
}

static int Lfile_length(lua_State *L) {
    PHYSFS_File *file = check_file(L, 1);
    PHYSFS_sint64 len = PHYSFS_fileLength(file);
    if (len < 0) return push_error(L, "length");
    lua_pushinteger(L, (lua_Integer)len);
    return 1;
}

static int Lfile_len(lua_State *L) {
    PHYSFS_File *file = check_file(L, 1);
    PHYSFS_sint64 len = PHYSFS_fileLength(file);
    if (len < 0) { push_error(L, "length"), lua_error(L); }
    lua_pushinteger(L, (lua_Integer)len);
    return 1;
}

static int Lfile_seek(lua_State *L) {
    api("seek", seek(check_file(L, 1),
                (PHYSFS_uint64)luaL_checkinteger(L, 2)));
    return_self(L);
}

static int Lfile_buffSize(lua_State *L) {
    api("bufSize", setBuffer(check_file(L, 1),
                (PHYSFS_uint64)luaL_checkinteger(L, 2)));
    return_self(L);
}

static int read_format(lua_State *L, int idx, const char *p) {
    int sig = 1, little = 1, wide = 4;
    luaL_argcheck(L, *p != '\0', idx, "invalid empty format");
    if (*p == '<') little = 1, ++p;
    if (*p == '>') little = 0, ++p;
    if (*p == '1' || *p == '2' || *p == '4' || *p == '8')
       wide = *p - '0', ++p;
    if (*p == 'i') sig = 1, ++p;
    if (*p == 'u') sig = 0, ++p;
    if (*p != '\0') {
        lua_pushfstring(L, "invalid format char '%c'", *p);
        luaL_argerror(L, idx, lua_tostring(L, -1));
    }
    return (sig << 8) | (little << 4) | wide;
}

static int read_numbers(lua_State *L, PHYSFS_File *f, int fmt) {
    union {
        char b;
        PHYSFS_sint16 s16; PHYSFS_uint16 u16;
        PHYSFS_sint32 s32; PHYSFS_uint32 u32;
        PHYSFS_sint64 s64; PHYSFS_uint64 u64;
    } u;
    int r;
    if ((fmt&0xF) == 1) {
        if (PHYSFS_readBytes(f, &u.b, 1) != 1) return 0;
        lua_pushinteger(L, (fmt&0xF00) ? (signed char)u.b : (unsigned char)u.b);
        return 1;
    }
    switch (fmt) {
    default: return 0;
#define CASE(n,s,d) \
    case n: if ((r = PHYSFS_read##s(f, &u.d))) lua_pushinteger(L, u.d); break
    CASE(0x002, UBE16, u16); CASE(0x004, UBE32, u32); CASE(0x008, UBE64, u64);
    CASE(0x012, ULE16, u16); CASE(0x014, ULE32, u32); CASE(0x018, ULE64, u64);
    CASE(0x102, SBE16, s16); CASE(0x104, SBE32, s32); CASE(0x108, SBE64, s64);
    CASE(0x112, SLE16, s16); CASE(0x114, SLE32, s32); CASE(0x118, SLE64, s64);
#undef CASE
    }
    return r;
}

static int read_chars(lua_State *L, PHYSFS_File *f, size_t n) {
    luaL_Buffer B;
    size_t remain = n;
    luaL_buffinit(L, &B);
    while (remain > 0) {
        void *buff = luaL_prepbuffer(&B);
        size_t read = remain < LUAL_BUFFERSIZE ? remain : LUAL_BUFFERSIZE;
        PHYSFS_sint64 r = PHYSFS_readBytes(f, buff, read);
        if (r > 0) { luaL_addsize(&B, r); remain -= (size_t)r; continue; }
        if (remain < n) break;
        luaL_pushresult(&B);
        lua_pop(L, 1);
        return 0;
    }
    luaL_pushresult(&B);
    return 1;
}

static int Lfile_read(lua_State *L) {
    PHYSFS_File *file = check_file(L, 1);
    int r = 1, isint, i, top = lua_gettop(L);
    if (top == 1)
        return read_chars(L, file, ~(size_t)0) ? 1 : push_error(L, "read");
    for (i = 2; i <= top && r; ++i) {
        size_t size = (size_t)lua_tointegerx(L, i, &isint);
        if (isint)
            r = read_chars(L, file, size);
        else {
            const char *p = luaL_checkstring(L, i);
            if (*p == '*') p++;
            r = *p == 'a' ? read_chars(L, file, ~(size_t)0)
                          : read_numbers(L, file, read_format(L, i, p));
        }
        if (r) lua_replace(L, i);
    }
    lua_settop(L, i-1);
    if (!r) return i + push_error(L, "read") - 3;
    return top-1;
}

static int Lfile_write(lua_State *L) {
    PHYSFS_File *file = check_file(L, 1);
    int i, top = lua_gettop(L);
    PHYSFS_uint64 total = 0;
    for (i = 2; i <= top; ++i) {
        size_t len;
        const char *s = luaL_checklstring(L, i, &len);
        PHYSFS_sint64 r = PHYSFS_writeBytes(file, s, (PHYSFS_uint64)len);
        total += r;
        if (r < 0 || (size_t)r < len) {
            int e = push_error(L, "write");
            lua_pushinteger(L, (lua_Integer)total);
            return e+1;
        }
    }
    return_self(L);
}

static int write_numbers(PHYSFS_File *f, lua_Integer v, int fmt) {
    if ((fmt & 0xF) == 1) {
        char b = (fmt&0xF00) ? (signed char)v : (unsigned char)v;
        return PHYSFS_writeBytes(f, &b, 1) == 1;
    }
    switch (fmt) {
    default: return 0;
#define CASE(n,s,t) case n: return PHYSFS_write##s(f, (PHYSFS_##t)v)
        CASE(0x002, UBE16, uint16);
        CASE(0x004, UBE32, uint32);
        CASE(0x008, UBE64, uint64);
        CASE(0x012, ULE16, uint16);
        CASE(0x014, ULE32, uint32);
        CASE(0x018, ULE64, uint64);
        CASE(0x102, SBE16, sint16);
        CASE(0x104, SBE32, sint32);
        CASE(0x108, SBE64, sint64);
        CASE(0x112, SLE16, sint16);
        CASE(0x114, SLE32, sint32);
        CASE(0x118, SLE64, sint64);
#undef  CASE
    }
}

static int Lfile_writeInt(lua_State *L) {
    PHYSFS_File *file = check_file(L, 1);
    const char *p = luaL_checkstring(L, 2);
    int i, top = lua_gettop(L), fmt = read_format(L, 2, p);
    for (i = 3; i <= top; ++i) {
        lua_Integer v = luaL_checkinteger(L, i);
        if (!write_numbers(file, v, fmt)) {
            int e = push_error(L, "writeInt");
            lua_pushinteger(L, i/2);
            return e+1;
        }
    }
    return_self(L);
}

static void open_file(lua_State *L) {
    luaL_Reg libs[] = {
        { "__gc",       Lfile_close    },
        { "__len",      Lfile_len      },
        { "__tostring", Lfile_tostring },
#define ENTRY(name) { #name, Lfile_##name }
        ENTRY(close),
        ENTRY(eof),
        ENTRY(tell),
        ENTRY(seek),
        ENTRY(length),
        ENTRY(buffSize),
        ENTRY(flush),
        ENTRY(read),
        ENTRY(write),
        ENTRY(writeInt),
#undef  ENTRY
        { NULL, NULL }
    };
    if (luaL_newmetatable(L, LFS_FILE)) {
        luaL_setfuncs(L, libs, 0);
        lua_pushvalue(L, -1);
        lua_setfield(L, -2, "__index");
    }
}


/* physfs loader */

static const char *checkfile(lua_State *L, const char *mod, PHYSFS_File **f) {
    const char *name = luaL_gsub(L, mod, ".", PHYSFS_getDirSeparator());
    luaL_Buffer B;
    if ((*f = PHYSFS_openRead(name)) != NULL) return name;
    lua_pushfstring(L, "%s.lua", name), name = lua_tostring(L, -1);
    if ((*f = PHYSFS_openRead(name)) != NULL) return name;
    lua_pushfstring(L, "%s.luac", name), name = lua_tostring(L, -1);
    if ((*f = PHYSFS_openRead(name)) != NULL) return name;
    lua_pop(L, 2), name = lua_tostring(L, -1);
    luaL_buffinit(L, &B);
    lua_pushfstring(L, "\n\tno file '%s' in physfs search path", name);
    luaL_addvalue(&B);
    lua_pushfstring(L, "\n\tno file '%s.lua' in physfs search path", name);
    luaL_addvalue(&B);
    lua_pushfstring(L, "\n\tno file '%s.luac' in physfs search path", name);
    luaL_addvalue(&B);
    luaL_pushresult(&B);
    return NULL;
}

static int loaderror(lua_State *L, const char *name) {
    return luaL_error(L, "error loading module '%s' from file '%s':\n"
            "\tphysfs: %s", lua_tostring(L, 1), name, lua_tostring(L, -1));
}

static int Ltryload(lua_State *L) {
    PHYSFS_File *f = (PHYSFS_File*)lua_touserdata(L, 1);
    const char *name = lua_tostring(L, 2);
    if (read_chars(L, f, ~(size_t)0)) {
        size_t len;
        const char *s = lua_tolstring(L, -1, &len);
        lua_pushfstring(L, "@%s", name);
        if (luaL_loadbuffer(L, s, len, lua_tostring(L, -1)) != LUA_OK)
            return loaderror(L, name);
        lua_pushvalue(L, 2);
        return 2;
    }
    lua_pushstring(L, PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
    return loaderror(L, name);
}

static int Lphysfs_loader(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    PHYSFS_File *f;
    int r;
    if (!PHYSFS_isInit()) {
        lua_pushstring(L, "physfs not init");
        return 1;
    }
    if ((name = checkfile(L, name, &f)) == NULL)
        return 1;
    lua_pushcfunction(L, Ltryload);
    lua_pushlightuserdata(L, f);
    lua_pushvalue(L, -3);
    r = lua_pcall(L, 2, 2, 0);
    PHYSFS_close(f);
    if (r != LUA_OK) lua_error(L);
    return 2;
}

static void open_loader(lua_State *L) {
    int pop = 1, r = lua53_getglobal(L, "package") == LUA_TTABLE
        && ((++pop, lua53_getfield(L, -1, "searchers") == LUA_TTABLE)
                || (++pop, lua53_getfield(L, -2, "loaders") == LUA_TTABLE));
    if (r) {
        lua_pushcfunction(L, Lphysfs_loader);
        lua_rawseti(L, -2, lua_rawlen(L, -2) + 1);
    }
    lua_pop(L, pop);
}


/* physfs routines */

static int push_list(lua_State *L, char **list, const char *fn);

static int Lfiles(lua_State *L)
{ return push_list(L, PHYSFS_enumerateFiles(luaL_checkstring(L, 1)), "files"); }

static int LcdRomDirs(lua_State *L)
{ return push_list(L, PHYSFS_getCdRomDirs(), "cdRomDirs"); }

static int LsearchPath(lua_State *L)
{ return push_list(L, PHYSFS_getSearchPath(), "searchPath"); }

static int Lmkdir(lua_State *L)
{ api("mkdir", mkdir(luaL_checkstring(L, 1))); return_self(L); }

static int Ldelete(lua_State *L)
{ api("delete", delete(luaL_checkstring(L, 1))); return_self(L); }

static int Lexists(lua_State *L)
{ lua_pushboolean(L, PHYSFS_exists(luaL_checkstring(L, 1))); return 1; }

static int LrealDir(lua_State *L)
{ lua_pushstring(L, PHYSFS_getRealDir(luaL_checkstring(L, 1))); return 1; }

static int LmountPoint(lua_State *L)
{ lua_pushstring(L, PHYSFS_getMountPoint(luaL_checkstring(L, 1))); return 1; }

static int Lunmount(lua_State *L)
{ api("unmount", unmount(luaL_checkstring(L, 1))); return 1; }

#define open_funcs(name)                                              \
    static int L##name(lua_State *L)              {                   \
        const char *s = luaL_checkstring(L, 1);                       \
        PHYSFS_File *f = PHYSFS_##name(s);                            \
        if (f == NULL) return push_error(L, #name);                   \
        *(PHYSFS_File**)lua_newuserdata(L, sizeof(PHYSFS_File*)) = f; \
        luaL_setmetatable(L, LFS_FILE); return 1; }
open_funcs(openRead)
open_funcs(openWrite)
open_funcs(openAppend)
#undef  open_funcs

static int push_list_helper(lua_State *L) {
    char **p = lua_touserdata(L, 1);
    int len = 0, count = 0;
    if (lua_istable(L, 2))
        len = lua_rawlen(L, 2);
    else {
        lua_settop(L, 1);
        lua_createtable(L, 0, 6);
    }
    for (; *p != NULL; ++p) {
        lua_pushstring(L, *p);
        lua_rawseti(L, 2, ++count + len);
    }
    return 1;
}

static int push_list(lua_State *L, char **list, const char *fn) {
    int r;
    if (list == NULL) return push_error(L, fn);
    lua_settop(L, 2);
    lua_pushcfunction(L, push_list_helper);
    lua_pushlightuserdata(L, list);
    lua_pushvalue(L, 2);
    r = lua_pcall(L, 2, 1, 0);
    PHYSFS_freeList(list);
    if (r == LUA_OK) return 1;
    lua_pushnil(L);
    lua_insert(L, -2);
    return 2;
}

static int LsaneConfig(lua_State *L) {
    const char *org = luaL_checkstring(L, 1);
    const char *app = luaL_checkstring(L, 2);
    const char *ext = luaL_optstring(L, 3, NULL);
    int includeCdRoms = lua_toboolean(L, 4);
    int archiveFirst  = lua_toboolean(L, 5);
    api("saneConfig", setSaneConfig(org, app, ext,
                includeCdRoms, archiveFirst));
    return_self(L);
}

static int LprefDir(lua_State *L) {
    const char *org = luaL_checkstring(L, 1);
    const char *app = luaL_checkstring(L, 2);
    const char *r = PHYSFS_getPrefDir(org, app);
    if (r == NULL) return push_error(L, "prefDir");
    lua_pushstring(L, r);
    return 1;
}

static int LwriteDir(lua_State *L) {
    if (lua_gettop(L) == 0) {
        lua_pushstring(L, PHYSFS_getWriteDir());
        return 1;
    }
    api("writeDir", setWriteDir(luaL_checkstring(L, 1)));
    return_self(L);
}

static int Lversion(lua_State *L) {
    PHYSFS_Version ver = { 0, 0, 0 };
    PHYSFS_getLinkedVersion(&ver);
    lua_pushinteger(L, ver.major);
    lua_pushinteger(L, ver.minor);
    lua_pushinteger(L, ver.patch);
    return 3;
}

static int LsupportedArchiveTypes(lua_State *L) {
    const PHYSFS_ArchiveInfo **i;
    int count = 0, len = 0;
    if (lua_istable(L, 1))
        len = lua_rawlen(L, 1);
    else {
        lua_settop(L, 0);
        lua_createtable(L, 10, 0);
    }
    for (i = PHYSFS_supportedArchiveTypes(); *i != NULL; ++i) {
        lua_createtable(L, 0, 5);
#define setf(t, v, f) lua_push##t(L, v), lua_setfield(L, -2, f)
        setf(string, (*i)->extension,   "ext");
        setf(string, (*i)->description, "desc");
        setf(string, (*i)->author,      "author");
        setf(string, (*i)->url,         "url");
        setf(boolean, (*i)->supportsSymlinks, "supportsSymlinks");
#undef setf
        lua_rawseti(L, 1, ++count + len);
    }
    lua_settop(L, 1);
    lua_pushinteger(L, count);
    return 2;
}

static int LuseSymlink(lua_State *L) {
    if (lua_gettop(L) == 0) {
        lua_pushboolean(L, PHYSFS_symbolicLinksPermitted());
        return 1;
    }
    PHYSFS_permitSymbolicLinks(lua_toboolean(L, 1));
    return 0;
}

static int LlastError(lua_State *L) {
    if (lua_gettop(L) == 0) {
        PHYSFS_ErrorCode code = PHYSFS_getLastErrorCode();
        const char *msg = "Unknown";
        switch (code) {
#define X(name) case PHYSFS_ERR_##name: msg = #name; break;
            error_codes(X)
#undef  X
        default: break;
        }
        lua_pushstring(L, msg);
        return 1;
    }
    else {
        static const char *opts[] = {
#define X(name) #name,
            error_codes(X)
#undef  X
                NULL
        };
        PHYSFS_ErrorCode code = luaL_checkoption(L, 1, NULL, opts);
        PHYSFS_setErrorCode(code);
        return 0;
    }
}

static int Lstat(lua_State *L) {
    const char *s = luaL_checkstring(L, 1);
    const char *type = "Unknown";
    PHYSFS_Stat buf;
    api("stat", stat(s, &buf));
    if (!lua_istable(L, 2)) {
        lua_settop(L, 1);
        lua_createtable(L, 0, 6);
    }
    switch (buf.filetype) {
    default: break;
    case PHYSFS_FILETYPE_REGULAR:   type = "file";    break;
    case PHYSFS_FILETYPE_DIRECTORY: type = "dir";     break;
    case PHYSFS_FILETYPE_SYMLINK:   type = "symlink"; break;
    case PHYSFS_FILETYPE_OTHER:     type = "other";   break;
    }
#define setf(t, v, f) lua_push##t(L, v), lua_setfield(L, 2, f)
    setf(string,  type, "type");
    setf(boolean, buf.readonly, "readonly");
    setf(integer, (lua_Integer)buf.filesize,   "size");
    setf(integer, (lua_Integer)buf.modtime,    "mtime");
    setf(integer, (lua_Integer)buf.createtime, "ctime");
    setf(integer, (lua_Integer)buf.accesstime, "atime");
#undef  setf
    lua_settop(L, 2);
    return 1;
}

static int Lmount(lua_State *L) {
    const char *dir = luaL_checkstring(L, 1);
    const char *point = luaL_optstring(L, 2, NULL);
    int prepend = lua_toboolean(L, 3);
    api("mount", mount(dir, point, prepend));
    return_self(L);
}

static int LmountFile(lua_State *L) {
    PHYSFS_File *file = check_file(L, 1);
    const char *name = luaL_checkstring(L, 2);
    const char *point = luaL_optstring(L, 3, NULL);
    int append = lua_toboolean(L, 4);
    api("mountFile", mountHandle(file, name, point, !append));
    *(PHYSFS_File**)lua_touserdata(L, 1) = NULL;
    return_self(L);
}

static int LmountMemory(lua_State *L) {
    size_t len;
    const char *s = luaL_checklstring(L, 1, &len);
    const char *name = luaL_checkstring(L, 2);
    const char *point = luaL_optstring(L, 3, NULL);
    int prepend = lua_toboolean(L, 4);
    void *data = malloc(len);
    if (data == NULL) luaL_error(L, "out of memory");
    memcpy(data, s, len);
    if (!PHYSFS_mountMemory(data, len, free, name, point, prepend)) {
        free(data);
        return push_error(L, "mountMemory");
    }
    return_self(L);
}

static int LconvInt(lua_State *L) {
    const char *p = luaL_checkstring(L, 1);
    int i, top = lua_gettop(L), fmt = read_format(L, 1, p);
    for (i = 2; i <= top; ++i) {
        lua_Integer v = luaL_checkinteger(L, i);
        switch (fmt) {
        default: break;
#define CASE(n,s,t) case n: \
            lua_pushinteger(L, (lua_Integer)PHYSFS_swap##s((PHYSFS_##t)v)); \
            lua_replace(L, i); break
        CASE(0x002, UBE16, uint16);
        CASE(0x004, UBE32, uint32);
        CASE(0x008, UBE64, uint64);
        CASE(0x012, ULE16, uint16);
        CASE(0x014, ULE32, uint32);
        CASE(0x018, ULE64, uint64);
        CASE(0x102, SBE16, sint16);
        CASE(0x104, SBE32, sint32);
        CASE(0x108, SBE64, sint64);
        CASE(0x112, SLE16, sint16);
        CASE(0x114, SLE32, sint32);
        CASE(0x118, SLE64, sint64);
#undef  CASE
        }
    }
    return top-1;
}

static const char *getarg0(lua_State *L) {
    const char *arg0 = NULL;
    int pop = 1;
    if (lua53_getglobal(L, "arg") == LUA_TTABLE) {
        lua_rawgeti(L, -1, 0);
        arg0 = lua_tostring(L, -1);
        ++pop;
    }
    lua_pop(L, pop);
    return arg0;
}

static int Ldeinit(lua_State *L) {
    (void)L;
    PHYSFS_deinit();
    return 0;
}

LUALIB_API int luaopen_physfs(lua_State *L) {
    luaL_Reg libs[] = {
        { "close", Lfile_close },
#define ENTRY(name) { #name, L##name }
        ENTRY(supportedArchiveTypes),
        ENTRY(version),
        ENTRY(saneConfig),
        ENTRY(prefDir),
        ENTRY(writeDir),
        ENTRY(cdRomDirs),
        ENTRY(searchPath),
        ENTRY(useSymlink),
        ENTRY(lastError),
        ENTRY(mkdir),
        ENTRY(delete),
        ENTRY(exists),
        ENTRY(realDir),
        ENTRY(stat),
        ENTRY(files),
        ENTRY(openRead),
        ENTRY(openWrite),
        ENTRY(openAppend),
        ENTRY(mount),
        ENTRY(mountPoint),
        ENTRY(mountFile),
        ENTRY(mountMemory),
        ENTRY(unmount),
        ENTRY(convInt),
#undef  ENTRY
        { NULL, NULL }
    };
    if (!PHYSFS_init(getarg0(L)))
        luaL_error(L, "can not init physfs library");
    open_file(L);
    open_loader(L);
    luaL_newlib(L, libs);
    lua_createtable(L, 0, 1);
    lua_pushcfunction(L, Ldeinit);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    lua_pushstring(L, PHYSFS_getBaseDir()), lua_setfield(L, -2, "baseDir");
    lua_pushstring(L, PHYSFS_getDirSeparator()), lua_setfield(L, -2, "dirSep");
    return 1;
}

/* win32cc: output="physfs.dll" libs='-llua54 libphysfs.a'
 * win32cc: flags+='-mdll -DLUA_BUILD_AS_DLL'
 * maccc: output="physfs.so" libs='libphysfs.a' flags+='-undefined dynamic_lookup'
 * maccc: flags+='-shared -framework CoreServices -framework IOKit'
 * cc: flags+='-Wextra -O3 -fprofile-arcs -ftest-coverage -pedantic -std=c89' */
