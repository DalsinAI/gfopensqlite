/* gfopensqlite: GFOpenBase's B0 probe (gfoffice docs/gfoffice/Design-OpenBase.md,
 * sections 3.2 and 11.3). One run answers what GFOpenBase needs to know about
 * this build of SQLite on the drawer DIR before any of GFOpenBase is written:
 *
 *   baseprobe DIR [ROWS]
 *
 * DIR is a drawer name ending in ':' or '/' ("DH1:Probe/", "RAM:", "T:").
 * ROWS sizes the benchmark (default 10000, as the design's table).
 *
 *   fresh    a database made new in DIR: create, insert, read back,
 *            integrity_check, closed and opened again (the RAM: issue: "file
 *            is not a database" on the first statement)
 *   fresh-*  the same with the "unix-none" VFS, and with journal_mode=TRUNCATE
 *   lock     two connections: A holds a write transaction, B without a busy
 *            wait is refused (SQLITE_BUSY), then writes once A commits
 *   none     a connection on "unix-none" leaves no "<database>.lck"
 *   journal  N commits of one row each with journal_mode DELETE, TRUNCATE and
 *            PERSIST: milliseconds, and the -journal file left after
 *   temp     a sort too big for a 16-page cache with temp_store=FILE, first
 *            with SQLite's own temporary folder, then sqlite3_temp_directory
 *            set to DIR
 *   fk       PRAGMA foreign_keys: an orphan refused, ON DELETE CASCADE
 *   meta     application_id 0x4F414442 and user_version kept; quick_check;
 *            VACUUM INTO a copy that opens
 *   real     REAL arithmetic and number text, compared with the host's
 *            answers (the soft-float question)
 *   bench    the design's six timings (11.3)
 *
 * Prints one line per check, "time" lines in milliseconds, and BASEPROBE PASS
 * or BASEPROBE FAIL; exit 0 on PASS. Builds on the host
 * (tests/host-baseprobe.sh) and for AmigaOS, hard or soft float.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>
#ifndef __amigaos__
#include <sys/time.h>
#endif

static int fails;
static char dir[256];
static int rows = 10000;

static long now_ms(void)
{
#ifdef __amigaos__
    return (long)(clock() * 1000.0 / CLOCKS_PER_SEC);
#else
    struct timeval tv; gettimeofday(&tv, NULL);
    return (long)(tv.tv_sec * 1000L + tv.tv_usec / 1000);
#endif
}

static void check(const char *what, int ok, const char *detail)
{
    printf("%-14s %s%s%s\n", what, ok ? "ok" : "FAIL", detail && *detail ? "  " : "", detail ? detail : "");
    if (!ok) fails++;
}

static void path(char *out, size_t n, const char *file)
{
    snprintf(out, n, "%s%s", dir, file);
}

static void zap(const char *file)
{
    char p[300], q[320];
    path(p, sizeof p, file);
    remove(p);
    snprintf(q, sizeof q, "%s-journal", p); remove(q);
    snprintf(q, sizeof q, "%s.lck", p); remove(q);
}

static long file_size(const char *p)
{
    FILE *f = fopen(p, "rb");
    long n;
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    n = ftell(f);
    fclose(f);
    return n;
}

static int exists(const char *p)
{
    FILE *f = fopen(p, "rb");
    if (f) fclose(f);
    return f != NULL;
}

static sqlite3 *open_db(const char *file, const char *vfs, int flags)
{
    char p[300];
    sqlite3 *db = NULL;
    path(p, sizeof p, file);
    if (sqlite3_open_v2(p, &db, flags ? flags : SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, vfs) != SQLITE_OK) {
        printf("               open %s: %s\n", p, db ? sqlite3_errmsg(db) : "out of memory");
        sqlite3_close(db);
        return NULL;
    }
    return db;
}

static int exec(sqlite3 *db, const char *sql, char *err, size_t n)
{
    char *e = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &e);
    if (err && n) snprintf(err, n, "%s", e ? e : "");
    sqlite3_free(e);
    return rc;
}

/* The first column of the first row of sql, as text, into out. */
static int one(sqlite3 *db, const char *sql, char *out, size_t n)
{
    sqlite3_stmt *st;
    int rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    out[0] = 0;
    if (rc != SQLITE_OK) { snprintf(out, n, "prepare: %s", sqlite3_errmsg(db)); return rc; }
    rc = sqlite3_step(st);
    if (rc == SQLITE_ROW) {
        const unsigned char *t = sqlite3_column_text(st, 0);
        snprintf(out, n, "%s", t ? (const char *)t : "NULL");
        rc = SQLITE_OK;
    } else if (rc == SQLITE_DONE) {
        rc = SQLITE_OK;
    } else {
        snprintf(out, n, "step: %s", sqlite3_errmsg(db));
    }
    sqlite3_finalize(st);
    return rc;
}

/* A database made new in DIR, written, read, closed and opened again. */
static void test_fresh(const char *label, const char *vfs, const char *journal)
{
    const char *file = "probe-fresh.oadb";
    char p[300], d[300], sql[800], got[400];
    sqlite3 *db;
    int rc;
    long sz;
    zap(file);
    path(p, sizeof p, file);
    if (!(db = open_db(file, vfs, 0))) { check(label, 0, "open failed"); return; }
    if (journal) {
        snprintf(sql, sizeof sql, "PRAGMA journal_mode=%s", journal);
        one(db, sql, got, sizeof got);
    }
    rc = exec(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT NOT NULL, n INTEGER);"
                  "INSERT INTO t(name, n) VALUES('amiga', 1200), ('base', 3);", d, sizeof d);
    if (rc) {
        sz = file_size(p);
        snprintf(got, sizeof got, "first statement rc=%d %s (file %ld bytes)", rc, d, sz);
        check(label, 0, got);
        sqlite3_close(db);
        return;
    }
    one(db, "SELECT group_concat(name || '=' || n, ',') FROM t ORDER BY id", got, sizeof got);
    sqlite3_close(db);
    sz = file_size(p);
    if (!(db = open_db(file, vfs, SQLITE_OPEN_READWRITE))) { check(label, 0, "reopen failed"); return; }
    one(db, "PRAGMA integrity_check", d, sizeof d);
    rc = !strcmp(got, "amiga=1200,base=3") && !strcmp(d, "ok");
    snprintf(sql, sizeof sql, "rows %s, integrity %s, %ld bytes", got, d, sz);
    one(db, "SELECT count(*) FROM t", got, sizeof got);
    rc = rc && !strcmp(got, "2");
    check(label, rc, sql);
    sqlite3_close(db);
    zap(file);
}

static void test_lock(void)
{
    const char *file = "probe-lock.oadb";
    sqlite3 *a, *b;
    char e[200];
    int rc;
    zap(file);
    a = open_db(file, NULL, 0);
    b = open_db(file, NULL, 0);
    if (!a || !b) { check("lock", 0, "open failed"); sqlite3_close(a); sqlite3_close(b); return; }
    exec(a, "CREATE TABLE t(x)", NULL, 0);
    sqlite3_busy_timeout(b, 0);
    exec(a, "BEGIN IMMEDIATE; INSERT INTO t VALUES(1)", NULL, 0);
    rc = exec(b, "INSERT INTO t VALUES(2)", e, sizeof e);
    check("lock-refused", rc == SQLITE_BUSY, rc == SQLITE_BUSY ? "B got SQLITE_BUSY" : e);
    exec(a, "COMMIT", NULL, 0);
    rc = exec(b, "INSERT INTO t VALUES(2)", e, sizeof e);
    one(b, "SELECT count(*) FROM t", e, sizeof e);
    check("lock-after", rc == SQLITE_OK && !strcmp(e, "2"), e);
    sqlite3_close(a);
    sqlite3_close(b);
    zap(file);
}

static void test_none(void)
{
    const char *file = "probe-none.oadb";
    char p[300], l[320];
    sqlite3 *db;
    zap(file);
    path(p, sizeof p, file);
    snprintf(l, sizeof l, "%s.lck", p);
    if (!(db = open_db(file, "unix-none", 0))) { check("none", 0, "open failed"); return; }
    exec(db, "CREATE TABLE t(x); INSERT INTO t VALUES(1)", NULL, 0);
    sqlite3_close(db);
    check("none-nolck", !exists(l), exists(l) ? "a .lck was made" : "no .lck");
    zap(file);
}

static void test_journal(const char *mode, int commits)
{
    const char *file = "probe-journal.oadb";
    char p[300], j[320], sql[100], got[100], msg[160];
    sqlite3 *db;
    sqlite3_stmt *st;
    long t0, t1;
    int i, rc = SQLITE_OK;
    zap(file);
    path(p, sizeof p, file);
    snprintf(j, sizeof j, "%s-journal", p);
    if (!(db = open_db(file, NULL, 0))) { check("journal", 0, "open failed"); return; }
    snprintf(sql, sizeof sql, "PRAGMA journal_mode=%s", mode);
    one(db, sql, got, sizeof got);
    exec(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, a TEXT, b INTEGER, c TEXT, d INTEGER)", NULL, 0);
    sqlite3_prepare_v2(db, "INSERT INTO t(a, b, c, d) VALUES('Customer name', ?1, 'Leeds', ?1 * 7)", -1, &st, NULL);
    t0 = now_ms();
    for (i = 0; i < commits && rc == SQLITE_OK; i++) {
        sqlite3_bind_int(st, 1, i);
        rc = sqlite3_step(st) == SQLITE_DONE ? SQLITE_OK : SQLITE_ERROR;
        sqlite3_reset(st);
    }
    t1 = now_ms();
    sqlite3_finalize(st);
    snprintf(msg, sizeof msg, "mode %s, %d commits, journal left: %s", got, commits, exists(j) ? "yes" : "no");
    check("journal", rc == SQLITE_OK, msg);
    printf("time journal-%-8s %6ld ms  (%ld ms a commit)\n", mode, t1 - t0, commits ? (t1 - t0) / commits : 0);
    sqlite3_close(db);
    zap(file);
}

static void test_temp(const char *label, int set_dir)
{
    const char *file = "probe-temp.oadb";
    sqlite3 *db;
    char e[400], got[320];
    int rc;
    zap(file);
    if (set_dir) sqlite3_temp_directory = sqlite3_mprintf("%s", dir);
    if (!(db = open_db(file, NULL, 0))) { check(label, 0, "open failed"); return; }
    exec(db, "PRAGMA cache_size=16; PRAGMA temp_store=FILE;"
             "CREATE TABLE t(a TEXT);"
             "WITH RECURSIVE c(i) AS (SELECT 1 UNION ALL SELECT i + 1 FROM c WHERE i < 3000)"
             " INSERT INTO t SELECT printf('%08d-padding-to-make-the-rows-longer-%d', (i * 7919) % 3001, i) FROM c", NULL, 0);
    rc = one(db, "SELECT count(*) FROM (SELECT a FROM t ORDER BY a DESC)", got, sizeof got);
    if (rc == SQLITE_OK)
        rc = exec(db, "CREATE TEMP TABLE s AS SELECT a FROM t ORDER BY a", e, sizeof e);
    else
        snprintf(e, sizeof e, "%s", got);
    snprintf(got + strlen(got), sizeof got - strlen(got), " rows sorted%s%s", rc ? ", " : "", rc ? e : "");
    check(label, rc == SQLITE_OK, got);
    sqlite3_close(db);
    if (set_dir) { sqlite3_free(sqlite3_temp_directory); sqlite3_temp_directory = NULL; }
    zap(file);
}

static void test_fk(void)
{
    const char *file = "probe-fk.oadb";
    sqlite3 *db;
    char e[200], got[100];
    int rc;
    zap(file);
    if (!(db = open_db(file, NULL, 0))) { check("fk", 0, "open failed"); return; }
    one(db, "PRAGMA foreign_keys", got, sizeof got);
    check("fk-default", !strcmp(got, "0"), got[0] == '0' ? "off until asked" : got);
    exec(db, "PRAGMA foreign_keys=ON;"
             "CREATE TABLE customers(id INTEGER PRIMARY KEY, name TEXT);"
             "CREATE TABLE orders(id INTEGER PRIMARY KEY, customer INTEGER NOT NULL"
             " REFERENCES customers(id) ON DELETE CASCADE, total INTEGER);"
             "INSERT INTO customers VALUES(1, 'Ann'), (2, 'Bob');"
             "INSERT INTO orders(customer, total) VALUES(1, 500), (1, 250), (2, 99);", NULL, 0);
    rc = exec(db, "INSERT INTO orders(customer, total) VALUES(7, 1)", e, sizeof e);
    check("fk-orphan", (rc & 0xff) == SQLITE_CONSTRAINT, e);
    exec(db, "DELETE FROM customers WHERE id = 1", NULL, 0);
    one(db, "SELECT count(*) FROM orders", got, sizeof got);
    check("fk-cascade", !strcmp(got, "1"), got);
    sqlite3_close(db);
    zap(file);
}

static void test_meta(void)
{
    const char *file = "probe-meta.oadb";
    sqlite3 *db;
    char got[200], p[300], sql[400];
    int ok;
    zap(file);
    zap("probe-copy.oadb");
    if (!(db = open_db(file, NULL, 0))) { check("meta", 0, "open failed"); return; }
    exec(db, "PRAGMA application_id=1329677378; PRAGMA user_version=1;"
             "CREATE TABLE oadb_meta(key TEXT PRIMARY KEY, value);"
             "INSERT INTO oadb_meta VALUES('title', 'Probe')", NULL, 0);
    sqlite3_close(db);
    if (!(db = open_db(file, NULL, SQLITE_OPEN_READWRITE))) { check("meta", 0, "reopen failed"); return; }
    one(db, "SELECT printf('%x', application_id) || ' v' || user_version FROM pragma_application_id, pragma_user_version", got, sizeof got);
    check("meta-header", !strcmp(got, "4f414442 v1"), got);
    one(db, "PRAGMA quick_check", got, sizeof got);
    check("meta-quick", !strcmp(got, "ok"), got);
    path(p, sizeof p, "probe-copy.oadb");
    {
        char *q = sqlite3_mprintf("VACUUM INTO %Q", p);
        snprintf(sql, sizeof sql, "%s", q);
        sqlite3_free(q);
    }
    ok = exec(db, sql, got, sizeof got) == SQLITE_OK;
    sqlite3_close(db);
    if (ok && (db = open_db("probe-copy.oadb", NULL, SQLITE_OPEN_READONLY))) {
        one(db, "SELECT value FROM oadb_meta WHERE key = 'title'", got, sizeof got);
        ok = !strcmp(got, "Probe");
        sqlite3_close(db);
    }
    check("meta-vacuum", ok, got);
    zap(file);
    zap("probe-copy.oadb");
}

/* REAL work, with the answers this SQLite gives on a PC (tests/host-baseprobe.sh:
 * IEEE double, round to nearest); the Amiga builds must give the same text. */
static void test_real(void)
{
    static const struct { const char *sql, *want; } t[] = {
        { "SELECT 0.1 + 0.2", "0.30000000000000004" },
        { "SELECT (0.1 + 0.2) = 0.3", "0" },
        { "SELECT sum(x) FROM (SELECT 0.1 AS x UNION ALL SELECT 0.1 UNION ALL SELECT 0.1"
          " UNION ALL SELECT 0.1 UNION ALL SELECT 0.1 UNION ALL SELECT 0.1 UNION ALL SELECT 0.1"
          " UNION ALL SELECT 0.1 UNION ALL SELECT 0.1 UNION ALL SELECT 0.1)", "1.0" },
        { "SELECT 3.2 + 605.1 + -1.5", "606.80000000000007" },
        { "SELECT printf('%.17g', 3.2 + 605.1 + -1.5)", "606.8000000000001" },
        { "SELECT round(2.675, 2)", "2.67" },
        { "SELECT printf('%.2f', 1234.5)", "1234.50" },
        { "SELECT CAST('1.5e3' AS REAL) * 2", "3000.0" },
        { "SELECT 1e308 * 10", "Inf" },
        { "SELECT 7.0 / 3", "2.3333333333333335" },
        { "SELECT CAST(9007199254740993 AS REAL)", "9007199254740992.0" },
        { "SELECT avg(x) FROM (SELECT 1 AS x UNION ALL SELECT 2 UNION ALL SELECT 4)", "2.3333333333333335" },
        { "SELECT 123456789 * 1000 / 7", "17636684142" },
    };
    sqlite3 *db;
    char got[100], msg[200];
    unsigned i;
    int bad = 0;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) { check("real", 0, "open :memory: failed"); return; }
    for (i = 0; i < sizeof t / sizeof t[0]; i++) {
        one(db, t[i].sql, got, sizeof got);
        if (strcmp(got, t[i].want)) {
            printf("               real %u: %s gave %s, want %s\n", i + 1, t[i].sql, got, t[i].want);
            bad++;
        }
    }
    snprintf(msg, sizeof msg, "%u answers, %d different from the PC's", (unsigned)(sizeof t / sizeof t[0]), bad);
    check("real", bad == 0, msg);
    sqlite3_close(db);
}

static void timed(sqlite3 *db, const char *label, const char *sql)
{
    sqlite3_stmt *st;
    long t0 = now_ms(), n = 0;
    int rc;
    rc = sqlite3_prepare_v2(db, sql, -1, &st, NULL);
    if (rc == SQLITE_OK) {
        while ((rc = sqlite3_step(st)) == SQLITE_ROW) n++;
        sqlite3_finalize(st);
    }
    if (rc != SQLITE_DONE && rc != SQLITE_OK) {
        check(label, 0, sqlite3_errmsg(db));
        return;
    }
    printf("time %-17s %6ld ms  (%ld rows out)\n", label, now_ms() - t0, n);
}

static void test_bench(void)
{
    const char *file = "probe-bench.oadb";
    sqlite3 *db;
    sqlite3_stmt *st;
    long t0;
    int i, customers = rows / 10 ? rows / 10 : 1, rc = SQLITE_OK;
    char got[100];
    zap(file);
    if (!(db = open_db(file, NULL, 0))) { check("bench", 0, "open failed"); return; }
    exec(db, "CREATE TABLE customers(id INTEGER PRIMARY KEY, name TEXT, town TEXT, joined TEXT, points INTEGER);"
             "CREATE TABLE orders(id INTEGER PRIMARY KEY, customer INTEGER, placed TEXT, pence INTEGER, note TEXT)", NULL, 0);

    /* 1: insert ROWS rows of five fields in one transaction */
    t0 = now_ms();
    exec(db, "BEGIN", NULL, 0);
    sqlite3_prepare_v2(db, "INSERT INTO orders(customer, placed, pence, note) VALUES(?1, ?2, ?3, ?4)", -1, &st, NULL);
    for (i = 0; i < rows && rc == SQLITE_OK; i++) {
        char d[16], note[40];
        snprintf(d, sizeof d, "2026-%02d-%02d", i % 12 + 1, i % 28 + 1);
        snprintf(note, sizeof note, "order %d for customer %d", i, i % customers);
        sqlite3_bind_int(st, 1, (i * 7) % customers + 1);
        sqlite3_bind_text(st, 2, d, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(st, 3, (i * 37) % 10000);
        sqlite3_bind_text(st, 4, note, -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(st) == SQLITE_DONE ? SQLITE_OK : SQLITE_ERROR;
        sqlite3_reset(st);
    }
    sqlite3_finalize(st);
    exec(db, "COMMIT", NULL, 0);
    if (rc != SQLITE_OK) { check("bench-insert", 0, sqlite3_errmsg(db)); sqlite3_close(db); zap(file); return; }
    printf("time %-17s %6ld ms  (%d rows, one transaction)\n", "insert", now_ms() - t0, rows);

    exec(db, "BEGIN", NULL, 0);
    sqlite3_prepare_v2(db, "INSERT INTO customers(name, town, joined, points) VALUES(?1, ?2, '2020-01-01', ?3)", -1, &st, NULL);
    for (i = 0; i < customers; i++) {
        static const char *towns[] = { "Leeds", "York", "Hull", "Bradford", "Wakefield" };
        char name[32];
        snprintf(name, sizeof name, "Customer %05d", (i * 7919) % customers);
        sqlite3_bind_text(st, 1, name, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(st, 2, towns[i % 5], -1, SQLITE_STATIC);
        sqlite3_bind_int(st, 3, i % 100);
        sqlite3_step(st);
        sqlite3_reset(st);
    }
    sqlite3_finalize(st);
    exec(db, "COMMIT", NULL, 0);

    /* 2: 1,000 lookups by key */
    t0 = now_ms();
    sqlite3_prepare_v2(db, "SELECT note FROM orders WHERE id = ?1", -1, &st, NULL);
    for (i = 0; i < 1000; i++) {
        sqlite3_bind_int(st, 1, (i * 97) % rows + 1);
        if (sqlite3_step(st) != SQLITE_ROW) rc = SQLITE_ERROR;
        sqlite3_reset(st);
    }
    sqlite3_finalize(st);
    check("bench-lookup", rc == SQLITE_OK, "1000 found");
    printf("time %-17s %6ld ms  (1000 lookups by key)\n", "lookup", now_ms() - t0);

    /* 3: a scan with a WHERE on a field without an index */
    timed(db, "scan", "SELECT id FROM orders WHERE pence BETWEEN 5000 AND 5100");
    /* 4: a join grouped and summed */
    timed(db, "join-group-sum", "SELECT c.town, count(*), sum(o.pence) FROM orders o JOIN customers c ON c.id = o.customer GROUP BY c.town");
    /* 5: sort by a text field */
    timed(db, "sort-text", "SELECT note FROM orders ORDER BY note");
    /* 6: the same scan with an index, and a page of a datasheet by key */
    exec(db, "CREATE INDEX orders_pence ON orders(pence)", NULL, 0);
    timed(db, "scan-indexed", "SELECT id FROM orders WHERE pence BETWEEN 5000 AND 5100");
    timed(db, "page-by-key", "SELECT * FROM orders WHERE id > 5000 ORDER BY id LIMIT 100");
    timed(db, "page-by-offset", "SELECT * FROM orders ORDER BY id LIMIT 100 OFFSET 5000");

    one(db, "PRAGMA integrity_check", got, sizeof got);
    check("bench-integrity", !strcmp(got, "ok"), got);
    sqlite3_close(db);
    {
        char p[300];
        path(p, sizeof p, file);
        printf("size %-17s %6ld bytes\n", "bench-file", file_size(p));
    }
    zap(file);
}

int main(int argc, char **argv)
{
    long t0;
#if defined(__amigaos__) && defined(__HAVE_68881__)
    __asm__ volatile ("fmove.l %0,%%fpcr" : : "d" (0));   /* see sqlitetest.c */
#endif
    if (argc < 2) { printf("usage: baseprobe DIR [ROWS]\n"); return 20; }
    snprintf(dir, sizeof dir, "%s", argv[1]);
    if (dir[0] && dir[strlen(dir) - 1] != ':' && dir[strlen(dir) - 1] != '/' && strlen(dir) + 1 < sizeof dir)
        strcat(dir, "/");
    if (argc > 2) rows = atoi(argv[2]) > 0 ? atoi(argv[2]) : rows;
    printf("BASEPROBE sqlite %s vfs=%s threadsafe=%d dir=%s rows=%d float=%s\n",
           sqlite3_libversion(), sqlite3_vfs_find(NULL)->zName, sqlite3_threadsafe(), dir, rows,
#if defined(__HAVE_68881__) || !defined(__amigaos__)
           "hard"
#else
           "soft"
#endif
           );
    t0 = now_ms();
    test_fresh("fresh", NULL, NULL);
    test_fresh("fresh-none", "unix-none", NULL);
    test_fresh("fresh-truncate", NULL, "TRUNCATE");
    test_lock();
    test_none();
    test_journal("DELETE", 100);
    test_journal("TRUNCATE", 100);
    test_journal("PERSIST", 100);
    test_temp("temp-own", 0);
    test_temp("temp-dir", 1);
    test_fk();
    test_meta();
    test_real();
    test_bench();
    printf("time %-17s %6ld ms\n", "whole-probe", now_ms() - t0);
    printf("BASEPROBE %s\n", fails ? "FAIL" : "PASS");
    return fails ? 10 : 0;
}
