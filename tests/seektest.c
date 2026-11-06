/* gfopensqlite: patches/sqlite-3.53.4-amiga-seek.patch on the host
 * (tests/host-seektest.sh). lseek() refuses past the end of a file as
 * AmigaDOS does (tests/hostseek.h). Through the default VFS: a write past
 * the end grows the file with zeros, a read past the end is a short read
 * of zeros; then a database whose pages are written out of order stays
 * whole.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "sqlite3.h"

#undef lseek
long host_amiga_refused;

off_t host_amiga_lseek(int fd, off_t off, int whence)
{
    struct stat st;
    if (whence == SEEK_SET && fstat(fd, &st) == 0 && off > st.st_size) {
        ++host_amiga_refused;
        errno = EINVAL;
        return -1;
    }
    return lseek(fd, off, whence);
}

static int failures;

static void check(int ok, const char *what)
{
    printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) ++failures;
}

static int exec(sqlite3 *db, const char *sql)
{
    char *err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) printf("     %s: %s\n", sql, err ? err : "?");
    sqlite3_free(err);
    return rc;
}

static void vfs_cases(const char *dir)
{
    sqlite3_vfs *vfs = sqlite3_vfs_find(NULL);
    sqlite3_file *f = calloc(1, (size_t)vfs->szOsFile);
    char path[512], buf[300], data[100];
    sqlite3_int64 size = 0;
    int flags = 0, i, zero = 1, rc;
    snprintf(path, sizeof path, "%s/seek.bin", dir);
    unlink(path);
    rc = vfs->xOpen(vfs, path, f, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_MAIN_DB, &flags);
    check(rc == SQLITE_OK, "open a new file through the default VFS");
    if (rc != SQLITE_OK) return;
    memset(data, 0xA5, sizeof data);
    host_amiga_refused = 0;
    rc = f->pMethods->xWrite(f, data, sizeof data, 8192);
    check(rc == SQLITE_OK, "write 100 bytes at 8192 in an empty file");
    f->pMethods->xFileSize(f, &size);
    check(size == 8292, "the file is 8292 bytes");
    rc = f->pMethods->xRead(f, buf, 192, 8100);
    for (i = 0; i < 92; ++i) if (buf[i]) zero = 0;
    check(rc == SQLITE_OK && zero && (unsigned char)buf[92] == 0xA5 && (unsigned char)buf[191] == 0xA5,
          "the gap reads as zeros, the data as written");
    memset(buf, 0x55, sizeof buf);
    rc = f->pMethods->xRead(f, buf, 300, 100000);
    zero = 1;
    for (i = 0; i < 300; ++i) if (buf[i]) zero = 0;
    check(rc == SQLITE_IOERR_SHORT_READ && zero, "a read past the end: a short read of zeros");
    rc = f->pMethods->xRead(f, buf, 300, 8200);
    zero = 1;
    for (i = 92; i < 300; ++i) if (buf[i]) zero = 0;
    check(rc == SQLITE_IOERR_SHORT_READ && (unsigned char)buf[91] == 0xA5 && zero,
          "a read across the end: the data, then zeros");
    check(host_amiga_refused == 0, "no seek past the end reached lseek()");
    f->pMethods->xClose(f);
    free(f);
    unlink(path);
}

static void database_cases(const char *dir)
{
    sqlite3 *db;
    sqlite3_stmt *st;
    char path[512], sql[256];
    int i, rc, ok = 1;
    snprintf(path, sizeof path, "%s/seek.db", dir);
    unlink(path);
    rc = sqlite3_open(path, &db);
    check(rc == SQLITE_OK, "open a new database");
    if (rc) return;
    ok &= exec(db, "PRAGMA page_size=1024; PRAGMA cache_size=8; CREATE TABLE t(k INTEGER PRIMARY KEY, v BLOB);") == 0;
    for (i = 0; i < 400 && ok; ++i) {
        snprintf(sql, sizeof sql, "INSERT INTO t VALUES(%d, randomblob(%d));", (i * 7919) % 1000, 200 + i % 900);
        ok &= exec(db, sql) == 0;
    }
    ok &= exec(db, "BEGIN; UPDATE t SET v=randomblob(1500) WHERE k%3=0; DELETE FROM t WHERE k%5=0; COMMIT;") == 0;
    ok &= exec(db, "VACUUM;") == 0;
    check(ok, "400 inserts, a large update and a VACUUM with an 8-page cache");
    rc = sqlite3_prepare_v2(db, "PRAGMA integrity_check;", -1, &st, NULL);
    check(rc == SQLITE_OK && sqlite3_step(st) == SQLITE_ROW
          && !strcmp((const char *)sqlite3_column_text(st, 0), "ok"), "integrity_check: ok");
    sqlite3_finalize(st);
    check(host_amiga_refused == 0, "no seek past the end reached lseek()");
    sqlite3_close(db);
    unlink(path);
}

int main(int argc, char **argv)
{
    const char *dir = argc > 1 ? argv[1] : ".";
    vfs_cases(dir);
    database_cases(dir);
    printf("GFOPENSQLITE SEEK %s (%d failures)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
