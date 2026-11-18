/* tests/host-amigatest.sh: what sqlite.library does when it is loaded and
 * first opened (library/sqlite_lib.c), before the test's main(): the
 * library's pool, memory and mutexes given to SQLite.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "sqlite_amiga.h"

static void __attribute__((constructor)) host_amiga_start(void)
{
    sql_amiga_init();
    if (sql_amiga_start() != SQLITE_OK) {
        printf("host_amiga_start: SQLite would not take the library's memory and mutexes\n");
        exit(20);
    }
}
