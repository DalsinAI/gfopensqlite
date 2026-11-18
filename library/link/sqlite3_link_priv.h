/* libsqlite3_link.a's own header: sqlite.library's calls, and the program's
 * owner token. SQLITE_LINK_HOST is the host test (tests/host-libtest.sh):
 * the calls are plain C calls into the library's side there.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef SQLITE3_LINK_PRIV_H
#define SQLITE3_LINK_PRIV_H
#include <stdarg.h>
#ifdef SQLITE_LINK_HOST
#include "sqlite3.h"
typedef void *APTR;
typedef unsigned int ULONG;
#include "host_inline.h"
#else
#include <proto/sqlite.h>
#endif

/* what SQL_Attach answered for this program (sqlite3_link_open.c) */
extern APTR sqlite3_link_owner;

#endif
