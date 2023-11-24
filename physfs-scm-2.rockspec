package = "physfs"
version = "scm-2"

source = {
   url = "git://github.com/starwing/lua-physfs"
}

description = {
   homepage = "https://github.com/starwing/lua-physfs",
   license = "Lua License"
}

dependencies = {}

build = {
   type = "builtin",
   platforms = {
      macosx = {
         type = "command",
         build_command = "sh osxbuild.sh",
         install = { lib = { physfs = "physfs.so" } }
      }
   },
   modules = {
      physfs = {
         defines = {
            "PHYSFS_SUPPORTS_7Z",
            "PHYSFS_SUPPORTS_QPAK",
            "PHYSFS_SUPPORTS_GRP",
            "PHYSFS_SUPPORTS_HOG",
            "PHYSFS_SUPPORTS_MVL",
            "PHYSFS_SUPPORTS_WAD",
            "PHYSFS_SUPPORTS_SLB",
            "PHYSFS_SUPPORTS_ISO9660",
         },
         sources = {
            "lua-physfs.c",
            "one.c",
            "physfs_platform_android.c",
            "physfs_platform_haiku.cpp",
            "physfs_platform_os2.c",
            "physfs_platform_posix.c",
            "physfs_platform_qnx.c",
            "physfs_platform_unix.c",
            "physfs_platform_windows.c",
            "physfs_platform_winrt.cpp",
         }
      }
   }
}
