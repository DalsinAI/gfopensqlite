/* libsqlite3_link.a: sqlite.library opened for the program, and the
 * program's owner token (include/sqlite3_link.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/libraries.h>
#include <proto/exec.h>
#include "sqlite3_link_priv.h"
#include <sqlite3_link.h>

APTR sqlite3_link_owner;

int sqlite3_link_open(void)
{
    if (!SQLiteBase)
        SQLiteBase = OpenLibrary((CONST_STRPTR)SQLITELIB_NAME, SQLITELIB_VERSION);
    if (!SQLiteBase) return 0;
    if (!sqlite3_link_owner) sqlite3_link_owner = SQL_Attach();
    return 1;
}

void sqlite3_link_close(void)
{
    if (!SQLiteBase) return;
    if (sqlite3_link_owner) SQL_Detach(sqlite3_link_owner);
    sqlite3_link_owner = NULL;
    CloseLibrary(SQLiteBase);
    SQLiteBase = NULL;
}
