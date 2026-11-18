/* sqlite.library's calls for a program that opens the library itself:
 *   SQLiteBase = OpenLibrary(SQLITELIB_NAME, SQLITELIB_VERSION);
 *   owner = SQL_Attach();  ...  SQL_Open(name, &db, owner);  ...
 *   SQL_Detach(owner);  CloseLibrary(SQLiteBase);
 * Most programs link libsqlite3_link.a instead and call sqlite3_open().
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef PROTO_SQLITE_H
#define PROTO_SQLITE_H
#include <exec/types.h>
#include <exec/libraries.h>
#include <stdarg.h>
#include <sqlite3.h>
#include <libraries/sqlite.h>
extern struct Library *SQLiteBase;
#include <inline/sqlite.h>
#endif
