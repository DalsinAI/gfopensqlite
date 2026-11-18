/* SQLiteLibTest: SQLite through sqlite.library, by way of libsqlite3_link.a.
 * A plain SQLite program (sqlite3_open and the rest, as sqlite3.h has
 * them) that checks what crossing a library could break: 64-bit and
 * double values both ways, strings and blobs, calls from the library back
 * into the program (exec rows, SQL functions, an aggregate, a collation,
 * destructors, the progress, authorizer, busy, commit and update hooks),
 * the calls with a variable number of arguments, the lock between two
 * connections, a connection left open by a program that ends, and (on an
 * Amiga) a second task using its own connection and the first task's.
 *
 *   SQLiteLibTest [database] [notasks]      default T:sqlitelibtest.db
 *
 * Run it twice: it ends with a connection open and a write begun, on
 * purpose, and the second run passes only if the link library's exit
 * closed that connection. "notasks" leaves the second task out (vamos runs
 * one task).
 *
 * tests/host-libtest.sh runs the same file on this computer: the link
 * library's functions (under lnk_ names, tests/host_rename.h) call the
 * library's side (library/sqlite_lib_calls.h, sqlite_own.c) as plain C,
 * and that calls the real SQLite.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sqlite3.h>
#include "sqlite3_link_priv.h"      /* SQL_Attach, SQL_Open, SQL_Detach for the second program */

#ifndef SQLITE_LINK_HOST
#include <exec/types.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <sqlite3_link.h>
#define DEFAULT_DB "T:sqlitelibtest.db"
/* no __stack: with a Shell's small stack the library's own stack for a call is used */
#else
#define DEFAULT_DB "sqlitelibtest.db"
APTR sqlite3_link_owner;
#endif

static int checks, failed;

static void check(int ok, const char *what, int line)
{
    checks++;
    if (!ok) {
        failed++;
        printf("FAIL line %d: %s\n", line, what);
    }
}
#define CHECK(x) check((x) != 0, #x, __LINE__)

static sqlite3 *db;
static const char *db_name;

static int run(sqlite3 *d, const char *sql) { return sqlite3_exec(d, sql, 0, 0, 0); }

/* one number from one statement */
static sqlite3_int64 number(sqlite3 *d, const char *sql)
{
    sqlite3_stmt *s = 0;
    sqlite3_int64 v = -1;
    if (sqlite3_prepare_v2(d, sql, -1, &s, 0) == SQLITE_OK && sqlite3_step(s) == SQLITE_ROW)
        v = sqlite3_column_int64(s, 0);
    sqlite3_finalize(s);
    return v;
}

/* ---- values across the library ------------------------------------------ */

#define BIG ((sqlite3_int64)0x0123456789ABCDEFLL)

static void test_values(void)
{
    sqlite3_stmt *s = 0;
    static const unsigned char blob[6] = { 0, 1, 2, 0xfe, 0xff, 0 };
    const char *tail = 0;
    CHECK(run(db, "CREATE TABLE t(id INTEGER PRIMARY KEY, i, big, r, s, b, n)") == SQLITE_OK);
    CHECK(sqlite3_prepare_v2(db, "INSERT INTO t(i, big, r, s, b, n) VALUES(?1, ?2, ?3, ?4, ?5, ?6); SELECT 1",
                             -1, &s, &tail) == SQLITE_OK);
    CHECK(s != 0);
    CHECK(tail && !strcmp(tail, " SELECT 1"));
    CHECK(sqlite3_bind_parameter_count(s) == 6);
    CHECK(sqlite3_bind_parameter_index(s, "?3") == 3);
    CHECK(!strcmp(sqlite3_bind_parameter_name(s, 2), "?2"));
    CHECK(sqlite3_bind_int(s, 1, -42) == SQLITE_OK);
    CHECK(sqlite3_bind_int64(s, 2, BIG) == SQLITE_OK);
    CHECK(sqlite3_bind_double(s, 3, 3.25) == SQLITE_OK);
    CHECK(sqlite3_bind_text(s, 4, "amiga", -1, SQLITE_TRANSIENT) == SQLITE_OK);
    CHECK(sqlite3_bind_blob(s, 5, blob, sizeof blob, SQLITE_STATIC) == SQLITE_OK);
    CHECK(sqlite3_bind_null(s, 6) == SQLITE_OK);
    CHECK(sqlite3_stmt_readonly(s) == 0);
    CHECK(sqlite3_step(s) == SQLITE_DONE);
    CHECK(sqlite3_changes(db) == 1);
    CHECK(sqlite3_changes64(db) == 1);
    CHECK(sqlite3_last_insert_rowid(db) == 1);
    CHECK(sqlite3_reset(s) == SQLITE_OK);
    CHECK(sqlite3_clear_bindings(s) == SQLITE_OK);
    CHECK(sqlite3_bind_int64(s, 2, -BIG) == SQLITE_OK);
    CHECK(sqlite3_bind_text64(s, 4, "0123456789", 4, SQLITE_TRANSIENT, SQLITE_UTF8) == SQLITE_OK);
    CHECK(sqlite3_bind_blob64(s, 5, blob, 3, SQLITE_TRANSIENT) == SQLITE_OK);
    CHECK(sqlite3_bind_zeroblob(s, 6, 9) == SQLITE_OK);
    CHECK(sqlite3_step(s) == SQLITE_DONE);
    CHECK(sqlite3_finalize(s) == SQLITE_OK);
    CHECK(sqlite3_total_changes(db) == 2);
    CHECK(sqlite3_total_changes64(db) == 2);

    CHECK(sqlite3_prepare_v3(db, "SELECT i, big, r, s, b, n, r * 2 AS twice FROM t ORDER BY id", -1, 0, &s, 0) == SQLITE_OK);
    CHECK(sqlite3_column_count(s) == 7);
    CHECK(!strcmp(sqlite3_column_name(s, 6), "twice"));
    CHECK(sqlite3_stmt_readonly(s) != 0);
    CHECK(sqlite3_step(s) == SQLITE_ROW);
    CHECK(sqlite3_data_count(s) == 7);
    CHECK(sqlite3_stmt_busy(s) != 0);
    CHECK(sqlite3_column_type(s, 0) == SQLITE_INTEGER);
    CHECK(sqlite3_column_int(s, 0) == -42);
    CHECK(sqlite3_column_int64(s, 1) == BIG);
    CHECK(sqlite3_column_type(s, 2) == SQLITE_FLOAT);
    CHECK(sqlite3_column_double(s, 2) == 3.25);
    CHECK(sqlite3_column_double(s, 6) == 6.5);
    CHECK(sqlite3_column_type(s, 3) == SQLITE_TEXT);
    CHECK(!strcmp((const char *)sqlite3_column_text(s, 3), "amiga"));
    CHECK(sqlite3_column_bytes(s, 3) == 5);
    CHECK(sqlite3_column_type(s, 4) == SQLITE_BLOB);
    CHECK(sqlite3_column_bytes(s, 4) == 6);
    CHECK(!memcmp(sqlite3_column_blob(s, 4), blob, 6));
    CHECK(sqlite3_column_type(s, 5) == SQLITE_NULL);
    CHECK(sqlite3_step(s) == SQLITE_ROW);
    CHECK(sqlite3_column_int64(s, 1) == -BIG);
    CHECK(!strcmp((const char *)sqlite3_column_text(s, 3), "0123"));
    CHECK(sqlite3_column_bytes(s, 4) == 3);
    CHECK(sqlite3_column_bytes(s, 5) == 9);
    CHECK(sqlite3_step(s) == SQLITE_DONE);
    CHECK(sqlite3_db_handle(s) == db);
    CHECK(sqlite3_next_stmt(db, 0) == s);
    CHECK(strstr(sqlite3_sql(s), "ORDER BY id") != 0);
    CHECK(sqlite3_finalize(s) == SQLITE_OK);

    /* a rowid past 32 bits */
    CHECK(run(db, "INSERT INTO t(id, i) VALUES(5000000000, 7)") == SQLITE_OK);
    CHECK(sqlite3_last_insert_rowid(db) == (sqlite3_int64)5000000000LL);
    sqlite3_set_last_insert_rowid(db, (sqlite3_int64)-7000000000LL);
    CHECK(sqlite3_last_insert_rowid(db) == (sqlite3_int64)-7000000000LL);
    CHECK(number(db, "SELECT max(id) FROM t") == (sqlite3_int64)5000000000LL);
    /* REAL arithmetic inside the library (soft float there, fpuauto) */
    CHECK(number(db, "SELECT CAST(round(1e6 * (0.1 + 0.2)) AS INTEGER)") == 300000);
    CHECK(number(db, "SELECT CAST(1e15 / 4 AS INTEGER)") == (sqlite3_int64)250000000000000LL);
}

/* ---- calls back into the program ---------------------------------------- */

static int rows_seen;
static int row(void *arg, int n, char **v, char **names)
{
    if (arg == (void *)&rows_seen && n == 2 && v[0] && !strcmp(names[1], "i")) rows_seen++;
    return 0;
}

static void fn_twice(sqlite3_context *ctx, int n, sqlite3_value **v)
{
    int *calls = (int *)sqlite3_user_data(ctx);
    (*calls)++;
    if (n != 1) { sqlite3_result_error(ctx, "one argument", -1); return; }
    switch (sqlite3_value_type(v[0])) {
    case SQLITE_INTEGER: sqlite3_result_int64(ctx, sqlite3_value_int64(v[0]) * 2); break;
    case SQLITE_FLOAT: sqlite3_result_double(ctx, sqlite3_value_double(v[0]) * 2); break;
    case SQLITE_TEXT: {
        char *z = sqlite3_mprintf("%s%s", sqlite3_value_text(v[0]), sqlite3_value_text(v[0]));
        sqlite3_result_text(ctx, z, -1, sqlite3_free);
        break;
    }
    case SQLITE_NULL: sqlite3_result_null(ctx); break;
    default: sqlite3_result_value(ctx, v[0]);
    }
}

static void fn_len_step(sqlite3_context *ctx, int n, sqlite3_value **v)
{
    sqlite3_int64 *sum = (sqlite3_int64 *)sqlite3_aggregate_context(ctx, sizeof *sum);
    (void)n;
    if (sum) *sum += sqlite3_value_bytes(v[0]);
}

static void fn_len_final(sqlite3_context *ctx)
{
    sqlite3_int64 *sum = (sqlite3_int64 *)sqlite3_aggregate_context(ctx, 0);
    sqlite3_result_int64(ctx, sum ? *sum : 0);
}

static int destroyed;
static void destroy(void *p) { if (p) destroyed++; }

static int backwards(void *arg, int n1, const void *a, int n2, const void *b)
{
    int n = n1 < n2 ? n1 : n2, c = memcmp(b, a, (size_t)n);
    (void)arg;
    return c ? c : n2 - n1;
}

static int progress_calls;
static int progress(void *arg) { (void)arg; return ++progress_calls >= 3; }

static int auth(void *arg, int action, const char *a, const char *b, const char *c, const char *d)
{
    (void)arg; (void)b; (void)c; (void)d;
    return action == SQLITE_READ && a && !strcmp(a, "secret") ? SQLITE_DENY : SQLITE_OK;
}

static int commits, updates;
static sqlite3_int64 updated_row;
static int on_commit(void *arg) { (void)arg; commits++; return 0; }
static void on_update(void *arg, int op, const char *dbn, const char *tbl, sqlite3_int64 rowid)
{
    (void)arg; (void)op; (void)dbn;
    if (!strcmp(tbl, "t")) { updates++; updated_row = rowid; }
}

static void test_callbacks(void)
{
    sqlite3_stmt *s = 0;
    int calls = 0;
    char *err = 0, *text;
    rows_seen = 0;
    CHECK(sqlite3_exec(db, "SELECT id, i FROM t", row, &rows_seen, &err) == SQLITE_OK);
    CHECK(rows_seen == 3);
    CHECK(err == 0);
    CHECK(sqlite3_exec(db, "SELECT x FROM nowhere", row, 0, &err) == SQLITE_ERROR);
    CHECK(err && strstr(err, "nowhere"));
    sqlite3_free(err);

    CHECK(sqlite3_create_function(db, "twice", 1, SQLITE_UTF8, &calls, fn_twice, 0, 0) == SQLITE_OK);
    CHECK(number(db, "SELECT twice(21)") == 42);
    CHECK(number(db, "SELECT twice(3000000000)") == (sqlite3_int64)6000000000LL);
    CHECK(number(db, "SELECT twice(1.25) = 2.5") == 1);
    CHECK(number(db, "SELECT twice('ab') = 'abab'") == 1);
    CHECK(number(db, "SELECT twice(NULL) IS NULL") == 1);
    CHECK(calls == 5);
    CHECK(sqlite3_create_function_v2(db, "lensum", 1, SQLITE_UTF8, &destroyed, 0, fn_len_step, fn_len_final, destroy) == SQLITE_OK);
    CHECK(number(db, "SELECT lensum(s) FROM t") == 9);
    destroyed = 0;
    CHECK(sqlite3_create_function_v2(db, "lensum", 1, SQLITE_UTF8, 0, 0, 0, 0, 0) == SQLITE_OK);
    CHECK(destroyed == 1);

    CHECK(sqlite3_create_collation(db, "backwards", SQLITE_UTF8, 0, backwards) == SQLITE_OK);
    CHECK(sqlite3_create_collation_v2(db, "backwards2", SQLITE_UTF8, &destroyed, backwards, destroy) == SQLITE_OK);
    CHECK(run(db, "CREATE TABLE w(x); INSERT INTO w VALUES('a'), ('c'), ('b')") == SQLITE_OK);
    CHECK(sqlite3_prepare_v2(db, "SELECT x FROM w ORDER BY x COLLATE backwards", -1, &s, 0) == SQLITE_OK);
    CHECK(sqlite3_step(s) == SQLITE_ROW && !strcmp((const char *)sqlite3_column_text(s, 0), "c"));
    CHECK(sqlite3_finalize(s) == SQLITE_OK);

    /* a destructor of the program's, called by the library */
    destroyed = 0;
    text = sqlite3_mprintf("%s", "freed by the program");
    CHECK(sqlite3_prepare_v2(db, "SELECT length(?1)", -1, &s, 0) == SQLITE_OK);
    CHECK(sqlite3_bind_text(s, 1, text, -1, destroy) == SQLITE_OK);
    CHECK(sqlite3_step(s) == SQLITE_ROW && sqlite3_column_int(s, 0) == 20);
    CHECK(sqlite3_finalize(s) == SQLITE_OK);
    CHECK(destroyed == 1);
    sqlite3_free(text);

    /* the progress handler stops a long statement */
    progress_calls = 0;
    sqlite3_progress_handler(db, 100, progress, 0);
    CHECK(run(db, "WITH RECURSIVE c(n) AS (SELECT 1 UNION ALL SELECT n + 1 FROM c WHERE n < 100000) SELECT sum(n) FROM c") == SQLITE_INTERRUPT);
    CHECK(progress_calls == 3);
    sqlite3_progress_handler(db, 0, 0, 0);
    CHECK(number(db, "WITH RECURSIVE c(n) AS (SELECT 1 UNION ALL SELECT n + 1 FROM c WHERE n < 1000) SELECT sum(n) FROM c") == 500500);

    CHECK(run(db, "CREATE TABLE secret(x)") == SQLITE_OK);
    CHECK(sqlite3_set_authorizer(db, auth, 0) == SQLITE_OK);
    CHECK(sqlite3_prepare_v2(db, "SELECT x FROM secret", -1, &s, 0) == SQLITE_AUTH);
    CHECK(strstr(sqlite3_errmsg(db), "prohibited") != 0);
    CHECK(sqlite3_set_authorizer(db, 0, 0) == SQLITE_OK);
    CHECK(run(db, "SELECT x FROM secret") == SQLITE_OK);

    commits = updates = 0;
    CHECK(sqlite3_commit_hook(db, on_commit, 0) == 0);
    CHECK(sqlite3_update_hook(db, on_update, 0) == 0);
    CHECK(run(db, "INSERT INTO t(id, i) VALUES(6000000000, 1)") == SQLITE_OK);
    CHECK(commits == 1 && updates == 1);
    CHECK(updated_row == (sqlite3_int64)6000000000LL);
    sqlite3_commit_hook(db, 0, 0);
    sqlite3_update_hook(db, 0, 0);
    CHECK(run(db, "DELETE FROM t WHERE id = 6000000000") == SQLITE_OK);
    CHECK(updates == 1);
}

/* ---- the calls with a variable number of arguments ---------------------- */

static char *via_va(const char *fmt, ...)
{
    va_list ap;
    char *z;
    va_start(ap, fmt);
    z = sqlite3_vmprintf(fmt, ap);
    va_end(ap);
    return z;
}

static void test_varargs(void)
{
    char buf[40], *z;
    sqlite3_str *str;
    int on = -1;
    z = sqlite3_mprintf("%d|%s|%lld|%.2f|%q|%c", -5, "text", BIG, 2.5, "it's", 'x');
    CHECK(z && !strcmp(z, "-5|text|81985529216486895|2.50|it''s|x"));
    sqlite3_free(z);
    z = via_va("%Q %05d %g", "a'b", 42, 0.5);
    CHECK(z && !strcmp(z, "'a''b' 00042 0.5"));
    sqlite3_free(z);
    CHECK(sqlite3_snprintf(sizeof buf, buf, "%s-%d-%lld", "n", 7, (sqlite3_int64)-9000000000LL) == buf);
    CHECK(!strcmp(buf, "n-7--9000000000"));
    sqlite3_snprintf(5, buf, "%s", "truncated");
    CHECK(!strcmp(buf, "trun"));
    str = sqlite3_str_new(db);
    sqlite3_str_appendf(str, "%d+%d=%lld", 1, 2, (sqlite3_int64)3);
    sqlite3_str_appendall(str, " ok");
    sqlite3_str_append(str, "!!!!", 2);
    sqlite3_str_appendchar(str, 3, '.');
    CHECK(sqlite3_str_length(str) == 13);
    CHECK(sqlite3_str_errcode(str) == SQLITE_OK);
    CHECK(!strcmp(sqlite3_str_value(str), "1+2=3 ok!!..."));
    z = sqlite3_str_finish(str);
    CHECK(z && !strcmp(z, "1+2=3 ok!!..."));
    sqlite3_free(z);

    CHECK(sqlite3_db_config(db, SQLITE_DBCONFIG_DEFENSIVE, 1, &on) == SQLITE_OK);
    CHECK(on == 1);
    CHECK(sqlite3_db_config(db, SQLITE_DBCONFIG_DEFENSIVE, -1, &on) == SQLITE_OK);
    CHECK(on == 1);
    CHECK(sqlite3_db_config(db, SQLITE_DBCONFIG_DEFENSIVE, 0, (int *)0) == SQLITE_OK);
    CHECK(sqlite3_db_config(db, SQLITE_DBCONFIG_MAINDBNAME, "main") == SQLITE_OK);
    /* SQLite belongs to every program: one of them can't set it up or shut it down */
    CHECK(sqlite3_config(SQLITE_CONFIG_LOOKASIDE, 0, 0) == SQLITE_MISUSE);
    CHECK(sqlite3_config(SQLITE_CONFIG_SERIALIZED) == SQLITE_OK);
    CHECK(sqlite3_initialize() == SQLITE_OK);
    CHECK(sqlite3_shutdown() == SQLITE_OK);
    CHECK(number(db, "SELECT count(*) FROM t") == 3);
    CHECK(sqlite3_soft_heap_limit64(1000000) == 0);
}

/* ---- the rest of the table ----------------------------------------------- */

static int busy_calls;
static int busy(void *arg, int n) { (void)arg; busy_calls++; return n < 2; }

static void test_misc(void)
{
    sqlite3_stmt *s = 0;
    sqlite3_value *v;
    void *p;
    char *z;
    int a = -1, b = -1;
    CHECK(!strcmp(sqlite3_libversion(), SQLITE_VERSION));
    CHECK(sqlite3_libversion_number() == SQLITE_VERSION_NUMBER);
    CHECK(!strcmp(sqlite3_sourceid(), SQLITE_SOURCE_ID));
    CHECK(sqlite3_threadsafe() != 0);
#if !defined(SQLITE_LINK_HOST) || defined(SQL_AMIGA_HOST)
    CHECK(!strcmp(sqlite3_vfs_find(0)->zName, "unix-dotfile"));
    CHECK(sqlite3_vfs_find("unix-none") != 0);
#endif
    CHECK(sqlite3_vfs_find("no such vfs") == 0);
    CHECK(sqlite3_prepare_v2(db, "SELEKT 1", -1, &s, 0) == SQLITE_ERROR);
    CHECK(s == 0);
    CHECK(sqlite3_errcode(db) == SQLITE_ERROR);
    CHECK(sqlite3_extended_errcode(db) == SQLITE_ERROR);
    CHECK(strstr(sqlite3_errmsg(db), "syntax error") != 0);
    CHECK(sqlite3_error_offset(db) == 0);
    CHECK(!strcmp(sqlite3_errstr(SQLITE_BUSY), "database is locked"));
    CHECK(sqlite3_extended_result_codes(db, 1) == SQLITE_OK);
    CHECK(sqlite3_get_autocommit(db) != 0);
    CHECK(run(db, "BEGIN") == SQLITE_OK);
    CHECK(sqlite3_get_autocommit(db) == 0);
    CHECK(sqlite3_txn_state(db, 0) == SQLITE_TXN_NONE);
    CHECK(run(db, "ROLLBACK") == SQLITE_OK);
    CHECK(sqlite3_db_filename(db, "main") != 0);
    CHECK(sqlite3_db_readonly(db, "main") == 0);
    CHECK(sqlite3_limit(db, SQLITE_LIMIT_VARIABLE_NUMBER, -1) > 0);
    CHECK(sqlite3_complete("SELECT 1;") != 0);
    CHECK(sqlite3_complete("SELECT 1") == 0);
    CHECK(sqlite3_stricmp("Amiga", "aMIGA") == 0);
    CHECK(sqlite3_strnicmp("Amiga", "amigo", 4) == 0);
    CHECK(sqlite3_strglob("a*a", "amiga") == 0);
    CHECK(sqlite3_strlike("A%A", "amiga", 0) == 0);

    p = sqlite3_malloc(100);
    CHECK(p != 0);
    CHECK(sqlite3_msize(p) >= 100);
    p = sqlite3_realloc(p, 5000);
    CHECK(p != 0 && sqlite3_msize(p) >= 5000);
    p = sqlite3_realloc64(p, 60000);
    CHECK(p != 0 && sqlite3_msize(p) >= 60000);
    sqlite3_free(p);
    p = sqlite3_malloc64(70000);
    CHECK(p != 0);
    memset(p, 0x5a, 70000);
    sqlite3_randomness(16, p);
    sqlite3_free(p);
    CHECK(sqlite3_memory_used() >= 0);
    CHECK(sqlite3_memory_highwater(0) >= 0);

    /* a value kept past its row, and bound again */
    CHECK(sqlite3_prepare_v2(db, "SELECT big, ?1 FROM t WHERE id = 1", -1, &s, 0) == SQLITE_OK);
    CHECK(sqlite3_step(s) == SQLITE_ROW);
    v = sqlite3_value_dup(sqlite3_column_value(s, 0));
    CHECK(v != 0);
    CHECK(sqlite3_reset(s) == SQLITE_OK);
    CHECK(sqlite3_bind_value(s, 1, v) == SQLITE_OK);
    CHECK(sqlite3_step(s) == SQLITE_ROW);
    CHECK(sqlite3_column_int64(s, 1) == BIG);
    z = sqlite3_expanded_sql(s);
    CHECK(z && strstr(z, "81985529216486895"));
    sqlite3_free(z);
    CHECK(sqlite3_value_int64(v) == BIG);
    CHECK(sqlite3_value_numeric_type(v) == SQLITE_INTEGER);
    sqlite3_value_free(v);
    CHECK(sqlite3_finalize(s) == SQLITE_OK);

    /* no WAL in this build (as the static library): WebKit asks for it and
     * is told the journal mode it has; the WAL calls do nothing */
    CHECK(sqlite3_prepare_v2(db, "PRAGMA journal_mode = WAL", -1, &s, 0) == SQLITE_OK);
    CHECK(sqlite3_step(s) == SQLITE_ROW);
    CHECK(!strcmp((const char *)sqlite3_column_text(s, 0), "delete"));
    CHECK(sqlite3_finalize(s) == SQLITE_OK);
    CHECK(run(db, "PRAGMA journal_mode = DELETE") == SQLITE_OK);
    CHECK(sqlite3_wal_hook(db, 0, 0) == 0);
    CHECK(sqlite3_wal_checkpoint_v2(db, 0, SQLITE_CHECKPOINT_PASSIVE, &a, &b) == SQLITE_OK);
    CHECK(sqlite3_db_release_memory(db) == SQLITE_OK);
    CHECK(sqlite3_db_cacheflush(db) == SQLITE_OK);
    CHECK(sqlite3_file_control(db, "main", SQLITE_FCNTL_FILE_POINTER, &p) == SQLITE_OK);

    /* the clock: UTC from the Amiga's clock and the locale, and back */
    CHECK(number(db, "SELECT CAST(strftime('%Y', 'now') AS INTEGER)") >= 2024);
    CHECK(number(db, "SELECT datetime('now', 'localtime') IS NOT NULL") == 1);
    CHECK(number(db, "SELECT strftime('%Y-%m-%d %H:%M:%S', 951782400, 'unixepoch') = '2000-02-29 00:00:00'") == 1);
    sqlite3_interrupt(db);
    CHECK(sqlite3_is_interrupted(db) != 0);
    CHECK(run(db, "SELECT 1") == SQLITE_OK);
    CHECK(sqlite3_sleep(1) >= 0);
}

/* ---- two connections, and a program that ends without closing ----------- */

/* Does this system refuse a second exclusive Lock() on a file? AmigaDOS
 * does, and the database lock is built on it. vamos does not (and one of
 * its file handles does not see what another has written until it is
 * closed), so there the checks that need two connections to meet are left
 * out. */
static int locks_work(void)
{
#ifndef SQLITE_LINK_HOST
    BPTR a, b, f = Open((STRPTR)"T:sqlitelibtest.probe", MODE_NEWFILE);
    int works;
    if (f) Close(f);
    a = Lock((STRPTR)"T:sqlitelibtest.probe", EXCLUSIVE_LOCK);
    b = Lock((STRPTR)"T:sqlitelibtest.probe", EXCLUSIVE_LOCK);
    works = a && !b;
    if (b) UnLock(b);
    if (a) UnLock(a);
    DeleteFile((STRPTR)"T:sqlitelibtest.probe");
    return works;
#else
    return 1;
#endif
}

/* Whether the library still has a connection on an owner's list. Only the
 * host test can ask: sql_owned is not one of the library's calls. */
#ifdef SQLITE_LINK_HOST
extern int sql_owned(sqlite3 *db);
#define OWNED_IS(d, v) CHECK(sql_owned(d) == (v))
#else
#define OWNED_IS(d, v) ((void)0)
#endif

/* a function's destructor: SQLite calls it from inside sqlite3_close */
static sqlite3 *closing;
static int owned_in_close;
static void fn_watch(sqlite3_context *c, int n, sqlite3_value **v) { (void)n; (void)v; sqlite3_result_int(c, 1); }
static void seen_in_close(void *p)
{
#ifdef SQLITE_LINK_HOST
    owned_in_close = sql_owned(*(sqlite3 **)p);
#else
    owned_in_close = *(sqlite3 **)p == closing ? 0 : 1;     /* the destructor ran, with its argument */
#endif
}

static void test_connections(void)
{
    sqlite3 *two = 0, *left = 0;
    sqlite3_stmt *s = 0;
    APTR other;
    int locks = locks_work();
    if (!locks) printf("no exclusive Lock() on this system: the checks of two connections meeting are left out\n");
    CHECK(sqlite3_open_v2(db_name, &two, SQLITE_OPEN_READWRITE, 0) == SQLITE_OK);
    CHECK(sqlite3_busy_timeout(two, 0) == SQLITE_OK);
    CHECK(run(db, "BEGIN IMMEDIATE") == SQLITE_OK);
    busy_calls = 0;
    if (locks) {
        CHECK(run(two, "BEGIN IMMEDIATE") == SQLITE_BUSY);
        CHECK(sqlite3_busy_handler(two, busy, 0) == SQLITE_OK);
        CHECK(run(two, "BEGIN IMMEDIATE") == SQLITE_BUSY);
        CHECK(busy_calls == 3);
    }
    CHECK(sqlite3_busy_timeout(two, 0) == SQLITE_OK);
    CHECK(run(db, "COMMIT") == SQLITE_OK);
    if (locks) {
        CHECK(run(two, "BEGIN IMMEDIATE; INSERT INTO w VALUES('two'); COMMIT") == SQLITE_OK);
        CHECK(number(db, "SELECT count(*) FROM w WHERE x = 'two'") == 1);
    }
    /* a statement not finalized: the close is refused and the connection stays the program's */
    CHECK(sqlite3_prepare_v2(two, "SELECT 1", -1, &s, 0) == SQLITE_OK);
    CHECK(sqlite3_close(two) == SQLITE_BUSY);
    OWNED_IS(two, 1);
    CHECK(sqlite3_finalize(s) == SQLITE_OK);
    /* a close that goes through: the connection is off its owner's list
     * before SQLite frees it (seen from a destructor the close runs), so
     * no entry is left naming an address the next open may be given */
    closing = two;
    owned_in_close = -1;
    CHECK(sqlite3_create_function_v2(two, "watch", 0, SQLITE_UTF8, &closing, fn_watch, 0, 0, seen_in_close) == SQLITE_OK);
    OWNED_IS(two, 1);
    CHECK(sqlite3_close(two) == SQLITE_OK);
    CHECK(owned_in_close == 0);
    CHECK(sqlite3_close(0) == SQLITE_OK);

    /* a refused close in a program that then ends: the connection is still
     * that program's, so its detach closes it and rolls its write back */
    other = SQL_Attach();
    CHECK(SQL_Open(db_name, &left, other) == SQLITE_OK);
    CHECK(run(left, "BEGIN IMMEDIATE; INSERT INTO w VALUES('refused')") == SQLITE_OK);
    CHECK(sqlite3_prepare_v2(left, "SELECT x FROM w", -1, &s, 0) == SQLITE_OK);
    CHECK(sqlite3_close(left) == SQLITE_BUSY);
    OWNED_IS(left, 1);
    SQL_Detach(other);
    OWNED_IS(left, 0);
    CHECK(sqlite3_busy_timeout(db, 0) == SQLITE_OK);
    CHECK(run(db, "BEGIN IMMEDIATE") == SQLITE_OK);
    CHECK(run(db, "COMMIT") == SQLITE_OK);
    CHECK(number(db, "SELECT count(*) FROM w WHERE x = 'refused'") == 0);
    /* close_v2 with a statement still open: nobody's at once, gone when the statement is */
    other = SQL_Attach();
    left = 0;
    CHECK(SQL_Open(db_name, &left, other) == SQLITE_OK);
    CHECK(sqlite3_prepare_v2(left, "SELECT 1", -1, &s, 0) == SQLITE_OK);
    CHECK(sqlite3_close_v2(left) == SQLITE_OK);
    OWNED_IS(left, 0);
    SQL_Detach(other);
    CHECK(sqlite3_step(s) == SQLITE_ROW);
    CHECK(sqlite3_finalize(s) == SQLITE_OK);
    left = 0;

    /* another program (its own owner token) opens the database, begins
     * writing and ends without closing anything: its detach closes the
     * connection, the write is rolled back and the lock is free */
    other = SQL_Attach();
    CHECK(other != 0 && other != sqlite3_link_owner);
    CHECK(SQL_Open(db_name, &left, other) == SQLITE_OK);
    CHECK(run(left, "BEGIN IMMEDIATE; INSERT INTO w VALUES('left behind')") == SQLITE_OK);
    CHECK(sqlite3_prepare_v2(left, "SELECT x FROM w", -1, &s, 0) == SQLITE_OK);
    CHECK(sqlite3_step(s) == SQLITE_ROW);
    CHECK(sqlite3_busy_timeout(db, 0) == SQLITE_OK);
    if (locks) CHECK(run(db, "BEGIN IMMEDIATE") == SQLITE_BUSY);
    SQL_Detach(other);
    CHECK(run(db, "BEGIN IMMEDIATE") == SQLITE_OK);
    CHECK(run(db, "COMMIT") == SQLITE_OK);
    CHECK(number(db, "SELECT count(*) FROM w WHERE x = 'left behind'") == 0);
    SQL_Detach(other);              /* twice: nothing happens */
    CHECK(sqlite3_busy_timeout(db, 2000) == SQLITE_OK);
}

/* ---- a second task (Amiga only) ------------------------------------------ */

#ifndef SQLITE_LINK_HOST
static struct Task *parent;
static sqlite3 *shared;             /* opened by the first task */
static volatile int child_result;   /* 0 while it runs */

/* No C library here: it runs beside the first task's. Its own connection
 * for 50 rows, each its own transaction (so the two tasks take turns at the
 * lock), and one row through the first task's connection. */
static void child(void)
{
    sqlite3 *mine = 0;
    sqlite3_stmt *s = 0;
    int k, rc, ok = 1;
    rc = sqlite3_open(db_name, &mine);
    if (rc != SQLITE_OK) ok = 0;
    if (ok && sqlite3_prepare_v2(mine, "INSERT INTO both(who, n) VALUES('child', ?1)", -1, &s, 0) != SQLITE_OK) ok = 0;
    for (k = 0; ok && k < 50; k++) {
        sqlite3_bind_int(s, 1, k);
        if (sqlite3_step(s) != SQLITE_DONE) ok = 0;
        sqlite3_reset(s);
    }
    sqlite3_finalize(s);
    if (sqlite3_close(mine) != SQLITE_OK) ok = 0;
    if (ok && sqlite3_exec(shared, "INSERT INTO both(who, n) VALUES('shared', 1)", 0, 0, 0) != SQLITE_OK) ok = 0;
    Forbid();
    child_result = ok ? 1 : -1;
    Signal(parent, SIGBREAKF_CTRL_F);
}

static void test_tasks(void)
{
    sqlite3_stmt *s = 0;
    int k;
    CHECK(run(db, "CREATE TABLE both(who, n)") == SQLITE_OK);
    CHECK(sqlite3_open(db_name, &shared) == SQLITE_OK);
    parent = FindTask(NULL);
    child_result = 0;
    SetSignal(0, SIGBREAKF_CTRL_F);
    CHECK(CreateNewProcTags(NP_Entry, (ULONG)child, NP_Name, (ULONG)"SQLiteLibTest child",
                            NP_StackSize, 65536, TAG_DONE) != 0);
    CHECK(sqlite3_prepare_v2(db, "INSERT INTO both(who, n) VALUES('parent', ?1)", -1, &s, 0) == SQLITE_OK);
    for (k = 0; k < 50; k++) {
        sqlite3_bind_int(s, 1, k);
        CHECK(sqlite3_step(s) == SQLITE_DONE);
        sqlite3_reset(s);
    }
    CHECK(sqlite3_finalize(s) == SQLITE_OK);
    while (!child_result) Wait(SIGBREAKF_CTRL_F);
    Delay(5);                       /* the child is past its last instruction of ours */
    CHECK(child_result == 1);
    CHECK(number(db, "SELECT count(*) FROM both WHERE who = 'parent'") == 50);
    CHECK(number(db, "SELECT count(*) FROM both WHERE who = 'child'") == 50);
    CHECK(number(db, "SELECT count(*) FROM both WHERE who = 'shared'") == 1);
    /* the connection the child used is closed by this task */
    CHECK(sqlite3_close(shared) == SQLITE_OK);
}
#endif

int main(int argc, char **argv)
{
    sqlite3 *leftover = 0;
    char *journal;
    db_name = argc > 1 ? argv[1] : DEFAULT_DB;
#ifdef SQLITE_LINK_HOST
    sqlite3_link_owner = SQL_Attach();
#endif
    journal = sqlite3_mprintf("%s-journal", db_name);
    remove(db_name);
    remove(journal);
    sqlite3_free(journal);
    printf("SQLiteLibTest: SQLite %s through sqlite.library, database %s\n", sqlite3_libversion(), db_name);
    CHECK(sqlite3_open(db_name, &db) == SQLITE_OK);
    if (!db) { printf("FAIL: no database\n"); return 20; }
    test_values();
    test_callbacks();
    test_varargs();
    test_misc();
    test_connections();
#ifndef SQLITE_LINK_HOST
    if (argc > 2 && !strcmp(argv[2], "notasks")) printf("the second task's part is left out\n");
    else test_tasks();
#endif
    CHECK(number(db, "SELECT count(*) FROM pragma_integrity_check WHERE integrity_check = 'ok'") == 1);
    CHECK(sqlite3_close_v2(db) == SQLITE_OK);
    /* left open on purpose: the link library's exit closes it (on the host, the detach below) */
    CHECK(sqlite3_open(db_name, &leftover) == SQLITE_OK);
    CHECK(run(leftover, "BEGIN IMMEDIATE") == SQLITE_OK);
#ifdef SQLITE_LINK_HOST
    SQL_Detach(sqlite3_link_owner);
    {
        sqlite3 *after = 0;
        sqlite3_link_owner = SQL_Attach();
        CHECK(sqlite3_open(db_name, &after) == SQLITE_OK);
        sqlite3_busy_timeout(after, 0);
        CHECK(run(after, "BEGIN IMMEDIATE; COMMIT") == SQLITE_OK);
        CHECK(sqlite3_close(after) == SQLITE_OK);
        SQL_Detach(sqlite3_link_owner);
    }
#endif
    printf("%s: %d checks, %d failed\n", failed ? "SQLITELIBTEST FAIL" : "SQLITELIBTEST PASS", checks, failed);
    return failed ? 20 : 0;
}
