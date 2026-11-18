/* libsqlite3_link.a: the calls tools/gen-library.py's table can't make.
 *   - The ones with a variable number of arguments: a library call has
 *     registers, not "...", so the list is taken here, in the program,
 *     and crosses as a va_list (sqlite3_mprintf is sqlite3_vmprintf) or
 *     unpacked (sqlite3_db_config).
 *   - The ones that would change SQLite for every program using the
 *     library: sqlite3_config, sqlite3_shutdown, sqlite3_soft_heap_limit64.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "sqlite3_link_priv.h"

char *sqlite3_mprintf(const char *zFormat, ...)
{
    va_list ap;
    char *z;
    va_start(ap, zFormat);
    z = SQL_Vmprintf(zFormat, ap);
    va_end(ap);
    return z;
}

char *sqlite3_snprintf(int n, char *zBuf, const char *zFormat, ...)
{
    va_list ap;
    char *z;
    va_start(ap, zFormat);
    z = SQL_Vsnprintf(n, zBuf, zFormat, ap);
    va_end(ap);
    return z;
}

void sqlite3_str_appendf(sqlite3_str *p, const char *zFormat, ...)
{
    va_list ap;
    va_start(ap, zFormat);
    SQL_StrVappendf(p, zFormat, ap);
    va_end(ap);
}

/* Each option's arguments, as sqlite3.h gives them: a name; a buffer and
 * two numbers; or, for all the rest, an on/off number and where to put the
 * answer. */
int sqlite3_db_config(sqlite3 *db, int op, ...)
{
    va_list ap;
    void *p;
    int a = 0, b = 0;
    va_start(ap, op);
    switch (op) {
    case SQLITE_DBCONFIG_MAINDBNAME:
        p = va_arg(ap, char *);
        break;
    case SQLITE_DBCONFIG_LOOKASIDE:
        p = va_arg(ap, void *);
        a = va_arg(ap, int);
        b = va_arg(ap, int);
        break;
    default:
        a = va_arg(ap, int);
        p = va_arg(ap, int *);
        break;
    }
    va_end(ap);
    return SQL_DbConfig(db, op, p, a, b);
}

/* The library's SQLite belongs to every program that has it open, so one
 * program can't set it up. The three threading modes are answered
 * SQLITE_OK: the library runs serialized, which is safe for a program that
 * asked for any of them. Everything else (a program's own allocator, page
 * cache, log...) is SQLITE_MISUSE, which is what SQLite itself answers
 * once it is running, as it always is here. */
int sqlite3_config(int op, ...)
{
    switch (op) {
    case SQLITE_CONFIG_SINGLETHREAD:
    case SQLITE_CONFIG_MULTITHREAD:
    case SQLITE_CONFIG_SERIALIZED:
        return SQLITE_OK;
    default:
        return SQLITE_MISUSE;
    }
}

/* started when the library was opened */
int sqlite3_initialize(void) { return SQLITE_OK; }

/* the library shuts SQLite down itself, when its last program closes it */
int sqlite3_shutdown(void) { return SQLITE_OK; }

/* One heap for every program: a program can't put a limit on it. (The
 * static library's limit did nothing either: it was built, as this is,
 * without the memory statistics the limit works from.) */
sqlite3_int64 sqlite3_soft_heap_limit64(sqlite3_int64 n)
{
    (void)n;
    return 0;
}

/* Kept so a program that sets it still builds. The library's temporary
 * files go to T: whatever this says. */
char *sqlite3_temp_directory;
