/* openamigasqlite lock test: two connections to one database file.
 *
 *   locktest [-libnix-fcntl] DB [VFS]
 *
 * DB is a scratch file (deleted first). VFS, if given, is made the default as
 * a program would (sqlite3_vfs_register(..., 1)); without it the library's
 * default is used. -libnix-fcntl (host only) makes fcntl() fail as libnix's
 * does (-1, errno untouched), which is what the "unix" VFS meets on AmigaOS.
 *
 *   1  two writers: A holds BEGIN IMMEDIATE; B (no busy wait) must be refused.
 *   2  interleaved writers: A fills table t and B table u, each in its own
 *      transaction, B without waiting (when refused it rolls back and tries
 *      again after A's commit); the file must stay whole and hold every row
 *      that was committed.
 *   3  a reader waits: A holds a write transaction for about 300 ms; a second
 *      thread's SELECT on a connection with the library's own busy timeout
 *      must wait for the commit and see A's rows, not fail.
 *
 * Prints one line per test and LOCKTEST PASS or LOCKTEST FAIL; exit 0 on PASS.
 * Builds on the host (tests/host-locktest.sh) and for AmigaOS (link with
 * -lsqlite3 -lpthread).
 * MIT, Copyright (c) 2026 AmigaChrome Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sqlite3.h>
#ifndef __amigaos__
#include <sys/time.h>
#include <unistd.h>
#endif

static int fails;

static long now_ms(void)
{
#ifdef __amigaos__
    return (long)(clock() * 1000.0 / CLOCKS_PER_SEC);
#else
    struct timeval tv; gettimeofday(&tv, NULL);
    return (long)(tv.tv_sec * 1000L + tv.tv_usec / 1000);
#endif
}

static int exec(sqlite3 *db, const char *sql)
{
    return sqlite3_exec(db, sql, NULL, NULL, NULL);
}

static int count(sqlite3 *db, const char *sql, int *out)
{
    sqlite3_stmt *st; int rc;
    *out = -1;
    if ((rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL)) != SQLITE_OK) return rc;
    rc = sqlite3_step(st);
    if (rc == SQLITE_ROW) { *out = sqlite3_column_int(st, 0); rc = SQLITE_OK; }
    sqlite3_finalize(st);
    return rc;
}

static int integrity_ok(const char *name)
{
    sqlite3 *db; sqlite3_stmt *st; int ok = 0;
    if (sqlite3_open(name, &db) != SQLITE_OK) return 0;
    sqlite3_busy_timeout(db, 2000);
    if (sqlite3_prepare_v2(db, "PRAGMA integrity_check", -1, &st, NULL) == SQLITE_OK) {
        if (sqlite3_step(st) == SQLITE_ROW)
            ok = !strcmp((const char *)sqlite3_column_text(st, 0), "ok");
        sqlite3_finalize(st);
    }
    sqlite3_close(db);
    return ok;
}

static void fresh(const char *name)
{
    sqlite3 *db;
    char j[600];
    remove(name);
    snprintf(j, sizeof j, "%s-journal", name); remove(j);
    sqlite3_open(name, &db);
    exec(db, "CREATE TABLE t(who TEXT, x BLOB); CREATE TABLE u(who TEXT, x BLOB)");
    sqlite3_close(db);
}

/* 1: both writers "hold" the lock? */
static void test_two_writers(const char *name)
{
    sqlite3 *a, *b; int ra, rb;
    fresh(name);
    sqlite3_open(name, &a); sqlite3_open(name, &b);
    sqlite3_busy_timeout(b, 0);
    ra = exec(a, "BEGIN IMMEDIATE");
    rb = exec(b, "BEGIN IMMEDIATE");
    printf("TEST1 two_writers a=%d b=%d (%s)%s\n", ra, rb, sqlite3_errstr(rb),
           ra == SQLITE_OK && rb == SQLITE_OK ? " both hold the lock" : "");
    if (!(ra == SQLITE_OK && rb == SQLITE_BUSY)) fails++;
    exec(a, "COMMIT"); exec(b, "ROLLBACK");
    sqlite3_close(a); sqlite3_close(b);
}

/* 2: interleaved write transactions; B gives up when refused and retries
 * after A commits (as a careful program would). */
static void test_interleaved(const char *name)
{
    sqlite3 *a, *b; int i, ra = 0, rb = 0, rows = -1, want, whole, retried = 0;
    fresh(name);
    sqlite3_open(name, &a); sqlite3_open(name, &b);
    sqlite3_busy_timeout(a, 0); sqlite3_busy_timeout(b, 0);
    ra |= exec(a, "BEGIN");
    for (i = 0; i < 20; i++) ra |= exec(a, "INSERT INTO t VALUES('a', zeroblob(3000))");
    rb |= exec(b, "BEGIN");
    for (i = 0; i < 20 && rb == SQLITE_OK; i++) rb |= exec(b, "INSERT INTO u VALUES('b', zeroblob(3000))");
    ra |= exec(a, "COMMIT");
    if (rb == SQLITE_OK) rb = exec(b, "COMMIT");
    else {                       /* refused: roll back and do it again now A is done */
        exec(b, "ROLLBACK"); retried = 1;
        rb = exec(b, "BEGIN");
        for (i = 0; i < 20 && rb == SQLITE_OK; i++) rb |= exec(b, "INSERT INTO u VALUES('b', zeroblob(3000))");
        if (rb == SQLITE_OK) rb = exec(b, "COMMIT");
    }
    sqlite3_close(a); sqlite3_close(b);
    whole = integrity_ok(name);
    {
        sqlite3 *c; sqlite3_open(name, &c);
        sqlite3_busy_timeout(c, 2000);
        if (count(c, "SELECT (SELECT count(*) FROM t) + (SELECT count(*) FROM u)", &rows) != SQLITE_OK) rows = -1;
        sqlite3_close(c);
    }
    want = (ra == SQLITE_OK ? 20 : 0) + (rb == SQLITE_OK ? 20 : 0);
    printf("TEST2 interleaved a=%d b=%d b_retried=%d rows=%d want=%d integrity=%s\n",
           ra, rb, retried, rows, want, whole ? "ok" : "BAD");
    if (!whole || rows != want || ra != SQLITE_OK || rb != SQLITE_OK) fails++;
}

/* 3: a reader on another thread waits for the writer's commit */
struct reader { const char *name; int rc, rows; long waited; };
static void *reader_main(void *p)
{
    struct reader *r = p; sqlite3 *c; long t0;
    r->rc = sqlite3_open(r->name, &c);            /* the library's own busy timeout */
    t0 = now_ms();
    if (r->rc == SQLITE_OK) r->rc = count(c, "SELECT count(*) FROM t", &r->rows);
    r->waited = now_ms() - t0;
    sqlite3_close(c);
    return NULL;
}

static void test_reader_waits(const char *name)
{
    sqlite3 *a; pthread_t th; struct reader r; int ra;
    fresh(name);
    memset(&r, 0, sizeof r); r.name = name; r.rows = -1;
    sqlite3_open(name, &a);
    ra = exec(a, "BEGIN IMMEDIATE");
    ra |= exec(a, "INSERT INTO t VALUES('a', zeroblob(10))");
    ra |= exec(a, "INSERT INTO t VALUES('a', zeroblob(10))");
    ra |= exec(a, "INSERT INTO t VALUES('a', zeroblob(10))");
    pthread_create(&th, NULL, reader_main, &r);
    sqlite3_sleep(300);
    ra |= exec(a, "COMMIT");
    pthread_join(th, NULL);
    sqlite3_close(a);
    printf("TEST3 reader rc=%d (%s) rows=%d waited_ms=%ld writer=%d\n",
           r.rc, sqlite3_errstr(r.rc), r.rows, r.waited, ra);
    if (r.rc != SQLITE_OK || r.rows != 3 || ra != SQLITE_OK) fails++;
}

#ifndef __amigaos__
static int libnix_fcntl(int fd, int cmd, ...) { (void)fd; (void)cmd; return -1; }
#endif

int main(int argc, char **argv)
{
    int i = 1;
    const char *name, *vfs;
#if defined(__amigaos__) && defined(__HAVE_68881__)
    __asm__ volatile ("fmove.l %0,%%fpcr" : : "d" (0));   /* see sqlitetest.c */
#endif
    if (i < argc && !strcmp(argv[i], "-libnix-fcntl")) {
#ifndef __amigaos__
        sqlite3_vfs *u = sqlite3_vfs_find("unix");
        if (u && u->xSetSystemCall) u->xSetSystemCall(u, "fcntl", (sqlite3_syscall_ptr)libnix_fcntl);
#endif
        i++;
    }
    if (i >= argc) { printf("usage: locktest [-libnix-fcntl] DB [VFS]\n"); return 20; }
    name = argv[i++];
    vfs = i < argc ? argv[i] : NULL;
    if (vfs) sqlite3_vfs_register(sqlite3_vfs_find(vfs), 1);
    printf("LOCKTEST sqlite %s vfs=%s\n", sqlite3_libversion(), sqlite3_vfs_find(NULL)->zName);
    test_two_writers(name);
    test_interleaved(name);
    test_reader_waits(name);
    remove(name);
    printf("LOCKTEST %s\n", fails ? "FAIL" : "PASS");
    return fails ? 10 : 0;
}
