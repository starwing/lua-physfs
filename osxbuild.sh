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
   physfs/archiver_dir.c                       \
   physfs/archiver_grp.c                       \
   physfs/archiver_hog.c                       \
   physfs/archiver_iso9660.c                   \
   physfs/archiver_lzma.c                      \
   physfs/archiver_mvl.c                       \
   physfs/archiver_qpak.c                      \
   physfs/archiver_slb.c                       \
   physfs/archiver_unpacked.c                  \
   physfs/archiver_wad.c                       \
   physfs/archiver_zip.c                       \
   physfs/physfs.c                             \
   physfs/physfs_byteorder.c                   \
   physfs/physfs_unicode.c                     \
   physfs/platform_macosx.c                    \
   physfs/platform_posix.c                     \
   physfs/platform_unix.c                      \
   physfs/platform_windows.c                   \
   physfs/lzma/C/7zCrc.c                       \
   physfs/lzma/C/Archive/7z/7zBuffer.c         \
   physfs/lzma/C/Archive/7z/7zDecode.c         \
   physfs/lzma/C/Archive/7z/7zExtract.c        \
   physfs/lzma/C/Archive/7z/7zHeader.c         \
   physfs/lzma/C/Archive/7z/7zIn.c             \
   physfs/lzma/C/Archive/7z/7zItem.c           \
   physfs/lzma/C/Archive/7z/7zMethodID.c       \
   physfs/lzma/C/Compress/Branch/BranchX86.c   \
   physfs/lzma/C/Compress/Branch/BranchX86_2.c \
   physfs/lzma/C/Compress/Lzma/LzmaDecode.c

