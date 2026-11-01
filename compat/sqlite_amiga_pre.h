/* openamigasqlite: included ahead of sqlite3.c (-include), so the
 * amalgamation is built as it comes.
 *
 * The NDK's exec/types.h defines GLOBAL as extern. GFOpenNDK's sys/ioctl.h
 * reaches it (through sys/ioccom.h) from inside sqlite3.c, after SQLite has
 * defined its own GLOBAL(t,v), and replaces it. Including exec/types.h here
 * first and taking its GLOBAL away leaves SQLite's in place: the later
 * include finds EXEC_TYPES_H set and does nothing.
 *
 * MIT, Copyright (c) 2026 AmigaChrome Team. */
#ifndef SQLITE_AMIGA_PRE_H
#define SQLITE_AMIGA_PRE_H
#include <exec/types.h>
#undef GLOBAL
#endif
