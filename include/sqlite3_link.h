/* libsqlite3_link.a: SQLite's C API through sqlite.library.
 *
 * A program that includes <sqlite3.h> and links -lsqlite3_link where it
 * linked -lsqlite3 needs nothing from this file: the link library opens
 * sqlite.library before main() and closes it after, and when the library
 * is missing or too old the program stops at once, saying so (a requester
 * from Workbench, a line in the Shell).
 *
 * A program that wants to say it in its own words has a SQLiteBase of its
 * own, which keeps that start-up code out, and calls these:
 *
 *     struct Library *SQLiteBase;
 *     ...
 *     if (!sqlite3_link_open()) { my_requester("needs " SQLITE3_LINK_NEEDS); return 20; }
 *     ...
 *     sqlite3_link_close();
 *
 * No sqlite3_ call may be made before sqlite3_link_open() answers 1 or
 * after sqlite3_link_close().
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef SQLITE3_LINK_H
#define SQLITE3_LINK_H

#define SQLITE3_LINK_NEEDS "sqlite.library 1 or later in LIBS:"

#ifdef __cplusplus
extern "C" {
#endif

/* 1 when sqlite.library is open at the version this was built for. 0 when
 * it is missing or older; nothing is left open then. */
int sqlite3_link_open(void);
/* Closes the connections the program left open, then the library. Safe to
 * call when sqlite3_link_open answered 0, or twice. */
void sqlite3_link_close(void);

#ifdef __cplusplus
}
#endif
#endif
