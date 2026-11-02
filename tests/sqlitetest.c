/* gfopensqlite smoke test: create a table, insert, query, in T:. */
#include <stdio.h>
#include <sqlite3.h>
static int row(void *u, int n, char **v, char **c) { int i; (void)u; for (i = 0; i < n; i++) printf("%s=%s ", c[i], v[i] ? v[i] : "NULL"); printf("\n"); return 0; }
/* ROM mathieeesingbas.library leaves the FPU in single precision (FPCR $40)
 * in every task that opens it; doubles need FPCR 0. */
static void resetFPCR(void) { __asm__ volatile ("fmove.l %0,%%fpcr" : : "d" (0)); }

int main(int argc, char **argv)
{
    const char *name = argc > 1 ? argv[1] : "T:openamigasqlite-test.db";
    resetFPCR();
    sqlite3 *db; char *err = NULL; int rc;
    printf("SQLITE %s vfs=%s\n", sqlite3_libversion(), sqlite3_vfs_find(NULL)->zName);
    remove(name);
    rc = sqlite3_open(name, &db);
    if (rc) { printf("OPEN_FAIL %s\n", sqlite3_errmsg(db)); return 20; }
    rc = sqlite3_exec(db, "DROP TABLE IF EXISTS t; CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT, v REAL);"
        "INSERT INTO t(name, v) VALUES('amiga', 3.2), ('webkit', 605.1), ('cookie', -1.5);"
        "SELECT id, name, v, length(name) AS len FROM t ORDER BY v;", row, NULL, &err);
    if (rc) { printf("EXEC_FAIL %s\n", err); return 20; }
    rc = sqlite3_exec(db, "SELECT count(*) AS n, sum(v) AS total, upper('ok') AS u FROM t;", row, NULL, &err);
    sqlite3_close(db);
    if (argc < 3) remove(name);
    printf("SQLITE_DONE rc=%d\n", rc);
    return 0;
}
