#include "physfs/src/physfs.c"
#include "physfs/src/physfs_archiver_7z.c"
#include "physfs/src/physfs_archiver_csm.c"
#include "physfs/src/physfs_archiver_dir.c"
#include "physfs/src/physfs_archiver_grp.c"
#define readui32 hug_readui32
#include "physfs/src/physfs_archiver_hog.c"
#undef  readui32
#include "physfs/src/physfs_archiver_iso9660.c"
#include "physfs/src/physfs_archiver_mvl.c"
#include "physfs/src/physfs_archiver_qpak.c"
#include "physfs/src/physfs_archiver_slb.c"
#include "physfs/src/physfs_archiver_unpacked.c"
#define readui32 vdf_readui32
#include "physfs/src/physfs_archiver_vdf.c"
#undef  readui32
#include "physfs/src/physfs_archiver_wad.c"
#define readui32 zip_readui32
#include "physfs/src/physfs_archiver_zip.c"
#undef  readui32
#include "physfs/src/physfs_byteorder.c"
#include "physfs/src/physfs_unicode.c"

