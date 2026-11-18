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

/* db joins owner's connections, on the entry c when there is one (a close
 * that was refused puts its own back), else on a new one. An owner that is
 * not on the list (no token, or detached meanwhile) keeps nothing. */
static void tag(sqlite3 *db, void *owner, struct conn *c)
{
    struct owner *o;
    sqlite3_mutex *m = list_mutex();
    if (!db || !owner) { sqlite3_free(c); return; }
    if (!c && !(c = (struct conn *)sqlite3_malloc((int)sizeof *c))) return;
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

/* db leaves its owner's connections: its entry (the caller frees it or
 * hands it back to tag) and whose it was, or 0 when it was nobody's */
static struct conn *untag(sqlite3 *db, void **owner)
{
    struct owner *o;
    struct conn **p, *c = 0;
    sqlite3_mutex *m = list_mutex();
    *owner = 0;
    sqlite3_mutex_enter(m);
    for (o = owners; o && !c; o = o->next)
        for (p = &o->conns; *p; p = &(*p)->next)
            if ((*p)->db == db) {
                c = *p;
                *p = c->next;
                *owner = o;
                break;
            }
    sqlite3_mutex_leave(m);
    return c;
}

/* 1 while db is on an owner's list. For the tests. */
int sql_owned(sqlite3 *db)
{
    struct owner *o;
    struct conn *c;
    int found = 0;
    sqlite3_mutex *m = list_mutex();
    sqlite3_mutex_enter(m);
    for (o = owners; o && !found; o = o->next)
        for (c = o->conns; c && !found; c = c->next) found = c->db == db;
    sqlite3_mutex_leave(m);
    return found;
}

/* SQLite hands back a connection even when the open fails (for its error
 * text); the program closes that one too, so it is kept like any other. */
int sql_open(const char *filename, sqlite3 **ppDb, void *owner)
{
    int rc = sqlite3_open(filename, ppDb);
    if (ppDb) tag(*ppDb, owner, 0);
    return rc;
}

int sql_open_v2(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs, void *owner)
{
    int rc = sqlite3_open_v2(filename, ppDb, flags, zVfs);
    if (ppDb) tag(*ppDb, owner, 0);
    return rc;
}

/* The connection comes off its owner's list before SQLite closes it, never
 * after. Once sqlite3_close has freed it, another task's open can be handed
 * the same address, and an entry still on the list would then name that
 * other connection: the old owner's detach would close it under its new
 * program. A close SQLite refuses (statements not finalized) puts the entry
 * back, so the connection is still closed when its program ends. */
int sql_close(sqlite3 *db)
{
    void *owner;
    struct conn *c;
    int rc;
    if (!db) return sqlite3_close(db);
    c = untag(db, &owner);
    rc = sqlite3_close(db);
    if (rc == SQLITE_OK) sqlite3_free(c);
    else if (c) tag(db, owner, c);
    return rc;
}

/* sqlite3_close_v2 never refuses: with statements still open the connection
 * lives on until the last is finalized, as nobody's. */
int sql_close_v2(sqlite3 *db)
{
    void *owner;
    if (db) sqlite3_free(untag(db, &owner));
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
