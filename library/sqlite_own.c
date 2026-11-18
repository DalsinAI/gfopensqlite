/* sqlite.library: whose connection is whose.
 *
 * A program gets an owner token when it attaches (the link library's
 * start-up does it) and passes it with sqlite3_open and sqlite3_open_v2.
 * The library keeps each owner's open connections. When the program
 * detaches (the link library's exit does it), the connections it left open
 * are closed: their statements are finalized, their locks dropped and their
 * files closed, which AmigaOS would not do for a program that just ends.
 *
 * A token stands for the program, not for a task: any task of the program
 * may use or close a connection another of its tasks opened (the browser
 * opens databases on worker threads), under SQLite's own rules for that.
 *
 * Plain C over SQLite's own memory and mutex calls, so the same file runs
 * in the host test.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "sqlite_own.h"

struct conn {
    struct conn *next;
    sqlite3 *db;
};

struct owner {
    struct owner *next;
    struct conn *conns;
};

static struct owner *owners;

static sqlite3_mutex *list_mutex(void) { return sqlite3_mutex_alloc(SQLITE_MUTEX_STATIC_APP1); }

/* SQLite's memory has just gone (the library's last close): so has the list */
void sql_own_reset(void) { owners = 0; }

void *sql_attach(void)
{
    struct owner *o = (struct owner *)sqlite3_malloc((int)sizeof *o);
    sqlite3_mutex *m = list_mutex();
    if (!o) return 0;
    o->conns = 0;
    sqlite3_mutex_enter(m);
    o->next = owners;
    owners = o;
    sqlite3_mutex_leave(m);
    return o;
}

/* a connection nobody closed: statements first, or sqlite3_close refuses */
static void close_leftover(sqlite3 *db)
{
    sqlite3_stmt *s;
    while ((s = sqlite3_next_stmt(db, 0))) sqlite3_finalize(s);
    if (sqlite3_close(db) != SQLITE_OK) sqlite3_close_v2(db);
}

void sql_detach(void *owner)
{
    struct owner **p, *o = 0;
    struct conn *c;
    sqlite3_mutex *m = list_mutex();
    if (!owner) return;
    sqlite3_mutex_enter(m);
    for (p = &owners; *p; p = &(*p)->next)
        if (*p == owner) {
            o = *p;
            *p = o->next;
            break;
        }
    sqlite3_mutex_leave(m);
    if (!o) return;                 /* not a token, or detached already */
    while ((c = o->conns)) {
        o->conns = c->next;
        close_leftover(c->db);
        sqlite3_free(c);
    }
    sqlite3_free(o);
}

static void tag(sqlite3 *db, void *owner)
{
    struct owner *o;
    struct conn *c;
    sqlite3_mutex *m = list_mutex();
    if (!db || !owner) return;
    if (!(c = (struct conn *)sqlite3_malloc((int)sizeof *c))) return;
    c->db = db;
    sqlite3_mutex_enter(m);
    for (o = owners; o && o != owner; o = o->next) ;
    if (o) {
        c->next = o->conns;
        o->conns = c;
    }
    sqlite3_mutex_leave(m);
    if (!o) sqlite3_free(c);
}

static void untag(sqlite3 *db)
{
    struct owner *o;
    struct conn **p, *c = 0;
    sqlite3_mutex *m = list_mutex();
    sqlite3_mutex_enter(m);
    for (o = owners; o && !c; o = o->next)
        for (p = &o->conns; *p; p = &(*p)->next)
            if ((*p)->db == db) {
                c = *p;
                *p = c->next;
                break;
            }
    sqlite3_mutex_leave(m);
    sqlite3_free(c);
}

/* SQLite hands back a connection even when the open fails (for its error
 * text); the program closes that one too, so it is kept like any other. */
int sql_open(const char *filename, sqlite3 **ppDb, void *owner)
{
    int rc = sqlite3_open(filename, ppDb);
    if (rc == SQLITE_OK) sqlite3_busy_timeout(*ppDb, SQLITE_AMIGA_BUSY_MS);
    if (ppDb) tag(*ppDb, owner);
    return rc;
}

int sql_open_v2(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs, void *owner)
{
    int rc = sqlite3_open_v2(filename, ppDb, flags, zVfs);
    if (rc == SQLITE_OK) sqlite3_busy_timeout(*ppDb, SQLITE_AMIGA_BUSY_MS);
    if (ppDb) tag(*ppDb, owner);
    return rc;
}

int sql_close(sqlite3 *db)
{
    int rc = sqlite3_close(db);
    if (rc == SQLITE_OK && db) untag(db);
    return rc;
}

int sql_close_v2(sqlite3 *db)
{
    if (db) untag(db);
    return sqlite3_close_v2(db);
}

/* sqlite3_db_config's variable arguments, as the link library unpacked them */
int sql_db_config(sqlite3 *db, int op, void *p, int a, int b)
{
    switch (op) {
    case SQLITE_DBCONFIG_MAINDBNAME:
        return sqlite3_db_config(db, op, (const char *)p);
    case SQLITE_DBCONFIG_LOOKASIDE:
        return sqlite3_db_config(db, op, p, a, b);
    default:
        return sqlite3_db_config(db, op, a, (int *)p);
    }
}
