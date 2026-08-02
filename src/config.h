/* config.h — hand-written for ArcaOS / OS/2 port (replaces autoconf output) */
#ifndef CONFIG_H_OS2
#define CONFIG_H_OS2

#define HAVE_ATEXIT   1
#define HAVE_FLOOR    1
#define HAVE_POW      1
#define HAVE_SQRT     1
#define HAVE_INTTYPES_H 1
#define HAVE_MEMORY_H   1
#define HAVE_STDBOOL_H  1
#define HAVE_STDINT_H   1
#define HAVE_STDLIB_H   1
#define HAVE_STRING_H   1
#define HAVE_STRINGS_H  1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_UNISTD_H   1
#define STDC_HEADERS    1

#define HAVE_LIBSDL     1

#define PACKAGE         "kuklomenos"
#define PACKAGE_NAME    "Kuklomenos"
#define PACKAGE_TARNAME "kuklomenos"
#define PACKAGE_VERSION "0.4.5"
#define PACKAGE_STRING  "Kuklomenos 0.4.5"
#define PACKAGE_BUGREPORT ""
#define PACKAGE_URL     ""
#define VERSION         "0.4.5"

/* No libcurl on OS/2 — disable high score reporting */
/* #undef HAVE_LIBCURL */

#endif /* CONFIG_H_OS2 */
