/* OpenBrowser: POSIX calls SQLite's unix VFS uses that libnix lacks. AmigaOS
 * files have no Unix owner or mode bits to change, so these succeed and do
 * nothing. MIT, Copyright (c) 2026 AmigaChrome Team. */
#include <sys/types.h>
int fchmod(int fd, mode_t mode) { (void)fd; (void)mode; return 0; }
int fchown(int fd, uid_t owner, gid_t group) { (void)fd; (void)owner; (void)group; return 0; }

#ifdef __amigaos__
#include <string.h>
#include <proto/dos.h>

/* A database's whole name, for unixFullPathname (patches/sqlite-3.53.4-amiga-paths.patch).
 * A name with a volume or an assign ("DH1:x.db", "PROGDIR:Cookies.db") is whole
 * already: 0, and it is kept as given. A relative one ("Cookies.db", "data/x.db",
 * "/x.db", ":x.db") is its drawer's name from NameFromLock plus the file's: SQLite
 * names the -journal after it, and a relative journal name would be resolved
 * against whatever the current directory is when it is written or rolled back.
 * 0 when the drawer can't be locked or the name doesn't fit: kept as given. */
int sqlite_amiga_fullname(const char *name, char *out, int size)
{
    const char *colon = strchr(name, ':'), *file;
    char dir[512];
    BPTR l;
    int n, ok;
    if (!name[0] || (colon && colon != name)) return 0;
    file = (const char *)PathPart((STRPTR)name);    /* "data/x.db": "/x.db"; "/x.db" and "x.db": "x.db" */
    n = (int)(file - name);
    if (n >= (int)sizeof dir) return 0;
    memcpy(dir, name, n);
    dir[n] = 0;
    if (!(l = Lock((STRPTR)dir, ACCESS_READ))) return 0;    /* "" is the current directory */
    ok = NameFromLock(l, (STRPTR)out, size) && AddPart((STRPTR)out, FilePart((STRPTR)name), size);
    UnLock(l);
    return ok;
}
#endif

#if defined(__amigaos__) || defined(SQLITE_AMIGA_HOST_LOCKS)
/* The unix-dotfile VFS's lock on AmigaDOS (patches/sqlite-3.53.4-amiga-paths.patch):
 * an exclusive Lock() on the file "<database>.lck". Upstream's lock is a drawer,
 * "<database>.lock", made with mkdir and deleted at unlock; a reset or a crash in
 * the middle of a statement left it there, and every connection then got
 * "database is locked" until someone deleted it. A Lock() is gone after a reset,
 * and the ones still held at exit() are let go here (AmigaDOS doesn't free a
 * program's locks when it ends). One lock for readers and writers, as before.
 *   take   0: held. 1: busy (ERROR_OBJECT_IN_USE: another connection, in this
 *          program or another, holds it). -1: any other failure.
 *   drop   UnLock. The .lck file is never deleted: another connection may hold
 *          it, or be about to.
 * The drawer a build before this one left behind is deleted, if it is empty,
 * the first time a connection takes the new lock.
 * SQLITE_AMIGA_HOST_LOCKS: tests/host-locktest.sh, with tests/hostdos.h
 * standing in for dos.library. */
#include <stdlib.h>
#include <string.h>
#ifdef __amigaos__
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#else
#include "hostdos.h"
#endif

struct amiga_lck {
    struct amiga_lck *next;     /* on the held list while lock is set */
    BPTR lock;                  /* the exclusive Lock(), or 0 */
    int swept;                  /* the old drawer looked at */
    char *drawer;               /* "<database>.lock" */
    char name[1];               /* "<database>.lck" */
};
static struct amiga_lck *held;  /* under Forbid() */
static int exit_hooked;

/* At exit(): UnLock what connections that were never closed still hold. */
static void lck_release_all(void)
{
    struct amiga_lck *c;
    BPTR l;
    do {
        l = 0;
        Forbid();
        if ((c = held)) { held = c->next; l = c->lock; c->lock = 0; }
        Permit();
        if (l) UnLock(l);
    } while (c);
}

/* An empty "<database>.lock" drawer, from a build before this one, is deleted. */
static void lck_sweep(const char *drawer)
{
    struct FileInfoBlock *fib;
    BPTR l = Lock((STRPTR)drawer, SHARED_LOCK);
    int dir = 0;
    if (!l) return;                     /* none: the usual case */
    if ((fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL))) {
        dir = Examine(l, fib) && fib->fib_DirEntryType == ST_USERDIR;
        FreeDosObject(DOS_FIB, fib);
    }
    UnLock(l);
    if (dir) DeleteFile((STRPTR)drawer);    /* fails, harmlessly, unless it is empty */
}

void *sqlite_amiga_lck_new(const char *db)
{
    size_t n = strlen(db);
    struct amiga_lck *c = (struct amiga_lck *)malloc(sizeof *c + 2 * n + 10);
    if (!c) return 0;
    memset(c, 0, sizeof *c);
    memcpy(c->name, db, n);
    strcpy(c->name + n, ".lck");
    c->drawer = c->name + n + 5;
    memcpy(c->drawer, db, n);
    strcpy(c->drawer + n, ".lock");
    return c;
}

int sqlite_amiga_lck_take(void *ctx)
{
    struct amiga_lck *c = (struct amiga_lck *)ctx;
    BPTR l = Lock((STRPTR)c->name, EXCLUSIVE_LOCK);
    int hook;
    if (!l && IoErr() == ERROR_OBJECT_NOT_FOUND) {
        BPTR fh = Open((STRPTR)c->name, MODE_READWRITE);  /* made once, never deleted */
        if (fh) {
            Close(fh);
            l = Lock((STRPTR)c->name, EXCLUSIVE_LOCK);
        }
    }
    if (!l) return IoErr() == ERROR_OBJECT_IN_USE ? 1 : -1;
    Forbid();
    c->lock = l;
    c->next = held;
    held = c;
    hook = !exit_hooked;
    exit_hooked = 1;
    Permit();
    if (hook) atexit(lck_release_all);
    if (!c->swept) {
        c->swept = 1;
        lck_sweep(c->drawer);
    }
    return 0;
}

void sqlite_amiga_lck_drop(void *ctx)
{
    struct amiga_lck **p;
    BPTR l = 0;
    Forbid();
    for (p = &held; *p; p = &(*p)->next)
        if (*p == ctx) {
            *p = (*p)->next;
            l = ((struct amiga_lck *)ctx)->lock;
            ((struct amiga_lck *)ctx)->lock = 0;
            break;
        }
    Permit();
    if (l) UnLock(l);
}

/* 1 when another connection holds the lock (a shared Lock() is refused). */
int sqlite_amiga_lck_held(void *ctx)
{
    BPTR l = Lock((STRPTR)((struct amiga_lck *)ctx)->name, SHARED_LOCK);
    if (l) {
        UnLock(l);
        return 0;
    }
    return IoErr() == ERROR_OBJECT_IN_USE;
}

void sqlite_amiga_lck_free(void *ctx)
{
    if (!ctx) return;
    sqlite_amiga_lck_drop(ctx);
    free(ctx);
}
#endif

#ifdef SQLITE_EXTRA_AUTOEXT
/* Every new connection waits for a lock rather than failing at once
 * (build.sh: SQLITE_EXTRA_AUTOEXT=sqlite_amiga_autoext). The default VFS is
 * unix-dotfile, whose lock is one drawer, "<database>.lock", taken for reading
 * as well as writing: without a wait a second reader would get "database is
 * locked". A program can still set its own with sqlite3_busy_timeout(). */
#include "sqlite3.h"
#ifndef SQLITE_AMIGA_BUSY_MS
#define SQLITE_AMIGA_BUSY_MS 2000
#endif
int sqlite_amiga_autoext(sqlite3 *db)
{
    return sqlite3_busy_timeout(db, SQLITE_AMIGA_BUSY_MS);
}
#endif
