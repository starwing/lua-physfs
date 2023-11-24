#!/bin/sh

clang -O3 -fPIC -bundle -undefined dynamic_lookup \
   -framework CoreServices -framework IOKit -o physfs.so \
   -DPHYSFS_SUPPORTS_7Z      \
   -DPHYSFS_SUPPORTS_QPAK    \
   -DPHYSFS_SUPPORTS_GRP     \
   -DPHYSFS_SUPPORTS_HOG     \
   -DPHYSFS_SUPPORTS_MVL     \
   -DPHYSFS_SUPPORTS_WAD     \
   -DPHYSFS_SUPPORTS_SLB     \
   -DPHYSFS_SUPPORTS_ISO9660 \
   lua-physfs.c                                \
   one.c                                       \
   physfs/src/physfs_platform_apple.m          \
   physfs/src/physfs_platform_posix.c          \
   physfs/src/physfs_platform_unix.c           \
   physfs/src/physfs_platform_windows.c        \
