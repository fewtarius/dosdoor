/* config.h for dosdoor - generated manually */

#ifndef CONFIG_H
#define CONFIG_H 1

#ifdef __linux__
#define _GNU_SOURCE 1
#endif

#define VERSION_OF(a,b,c,d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))
#define IS_DEVEL_RELEASE (DOSEMU_VERSION_CODE && 65536)

#ifdef __ASM__
#define extern #
#else
#include "extern.h"

#include "confpath.h"
#define  DOSEMU_RC          ".dosdoorrc"
#define  OLD_DOS_RC         ".dosrc"
#define  LOCALDIR_BASE_NAME ".dosdoor"
#define  DOSEMU_CONF        "dosdoor.conf"
#define  DOSEMU_USERS       "dosdoor.users"
#define  DEFAULT_CONFIG_SCRIPT "builtin"
#define  DOSEMU_LOGLEVEL    "dosdoor.loglevel"
#define  DOSEMU_MIDI        "dosdoor-midi"
#define  DOSEMU_MIDI_IN     "dosdoor-midi_in"

EXTERN char *config_script_name INIT(DEFAULT_CONFIG_SCRIPT);
EXTERN char *config_script_path INIT(0);
EXTERN char *dosemu_users_file_path INIT("/etc/" DOSEMU_USERS);
EXTERN char *dosemu_loglevel_file_path INIT("/etc/" DOSEMU_LOGLEVEL);
EXTERN char *dosemu_rundir_path INIT("~/" LOCALDIR_BASE_NAME "/run");
EXTERN char *dosemu_localdir_path INIT("~/" LOCALDIR_BASE_NAME);

EXTERN char dosemulib_default[] INIT(DOSEMULIB_DEFAULT);
EXTERN char *dosemu_lib_dir_path INIT(dosemulib_default);
EXTERN char dosemuhdimage_default[] INIT(DOSEMUHDIMAGE_DEFAULT);
EXTERN char *dosemu_hdimage_dir_path INIT(dosemuhdimage_default);
EXTERN char keymaploadbase_default[] INIT(DOSEMULIB_DEFAULT "/");
EXTERN char *keymap_load_base_path INIT(keymaploadbase_default);
EXTERN char *keymap_dir_path INIT("keymap/");
EXTERN char *owner_tty_locks INIT("uucp");
EXTERN char *tty_locks_dir_path INIT("/var/lock");
EXTERN char *tty_locks_name_path INIT("LCK..");
EXTERN char *dexe_load_path INIT(dosemuhdimage_default);
EXTERN char *dosemu_midi_path INIT("~/" LOCALDIR_BASE_NAME "/run/" DOSEMU_MIDI);
EXTERN char *dosemu_midi_in_path INIT("~/" LOCALDIR_BASE_NAME "/run/" DOSEMU_MIDI_IN);

#define    DOSEMU_USERS_FILE     dosemu_users_file_path
#define    DOSEMU_LOGLEVEL_FILE  dosemu_loglevel_file_path
#define    RUNDIR                dosemu_rundir_path
#define    LOCALDIR              dosemu_localdir_path
#define    KEYMAP_LOAD_BASE_PATH keymap_load_base_path
#define    KEYMAP_DIR            keymap_dir_path
#define    OWNER_LOCKS           owner_tty_locks
#define    PATH_LOCKD            tty_locks_dir_path
#define    NAME_LOCKF            tty_locks_name_path
EXTERN char *dosemu_map_file_name INIT(0);
#define    DOSEMU_MAP_PATH       dosemu_map_file_name
#define    DOSEMU_MIDI_PATH      dosemu_midi_path
#define    DOSEMU_MIDI_IN_PATH   dosemu_midi_in_path

#endif /* not __ASM__ */

/* macOS platform */
#define CONFIG_HOST "darwin"
#define CONFIG_TIME __DATE__ " " __TIME__
#define DOSEMU_VERSION_CODE 0x01040001
#define EMUVER "dosdoor"
/* Compatibility defines for code that uses old dosemu version macros */
#ifndef VERSION
#define VERSION 1
#endif
#ifndef SUBLEVEL
#define SUBLEVEL 4
#endif
#ifndef PATCHLEVEL
#define PATCHLEVEL 0.1
#endif
#define VERDATE __DATE__
#define VERSTR "dosdoor"
#define PACKAGE_VERSION "dosdoor"
#define PACKAGE_BUGREPORT ""
#define PACKAGE_NAME "dosdoor"
#define PACKAGE_STRING "dosdoor"
#define PACKAGE_TARNAME "dosdoor"

/* GCC/Clang features */
#define GCC_VERSION_CODE 17000  /* approximate for Apple clang 17 */
#define RETSIGTYPE void
#define STDC_HEADERS 1

/* macOS has these */
#define HAVE_DIRENT_H 1
#define HAVE_GETTIMEOFDAY 1
#define HAVE_INTTYPES_H 1
#define HAVE_MEMORY_H 1
#define HAVE_SHM_OPEN 1
#define HAVE_SIGALTSTACK 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_STRUCT_STAT_ST_RDEV 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_UNISTD_H 1

/* macOS does NOT have these */
/* #undef HAVE_LIBRT */
/* #undef MAJOR_IN_SYSMACROS */
/* #undef MAJOR_IN_MKDEV */
/* #undef ASM_PEDANTIC */
/* #undef GASCODE16 */

/* Disabled features */
/* #undef ASPI_SUPPORT */
/* #undef HAVE_MITSHM */
/* #undef HAVE_XVIDMODE */
/* #undef USE_ALSA */
/* #undef USE_DL_PLUGINS */
/* #undef USE_GPM */
#define USE_MHPDBG 1
/* #undef USE_PTHREADS */
/* #undef USE_SBEMU */
/* #undef USE_SNDFILE */
/* #undef USE_SVGALIB */
/* #undef USING_NET */

/* Enabled features */
#define USE_SLANG 1
#define X86_EMULATOR 1
#define X_DISPLAY_MISSING 1

/* macOS compatibility */
#ifdef __APPLE__
#define _DARWIN_C_SOURCE 1
/* macOS doesn't have these Linux-specific features */
#define MAP_NORESERVE 0
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif
#endif

/* Plugin configuration */
#include "plugin_config.h"

#endif /* CONFIG_H */
