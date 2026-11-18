/* sqlite.library: whose connection is whose (sqlite_own.c), and the few
 * calls the library answers itself instead of passing straight to SQLite.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef SQLITE_OWN_H
#define SQLITE_OWN_H
#include "sqlite3.h"

void sql_own_reset(void);
void *sql_attach(void);
void sql_detach(void *owner);
int sql_open(const char *filename, sqlite3 **ppDb, void *owner);
int sql_open_v2(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs, void *owner);
int sql_close(sqlite3 *db);
int sql_close_v2(sqlite3 *db);
int sql_owned(sqlite3 *db);           /* 1 while db is on an owner's list: for the tests */
int sql_db_config(sqlite3 *db, int op, void *p, int a, int b);

#endif
