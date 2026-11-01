/* openamigasqlite: the dos.library calls compat/sqlite_amiga.c's lock uses,
 * emulated on the host for tests/host-locktest.sh (SQLITE_AMIGA_HOST_LOCKS).
 *
 * A Lock() is an open descriptor with flock() on it: EXCLUSIVE_LOCK is LOCK_EX,
 * SHARED_LOCK is LOCK_SH, and a clash is ERROR_OBJECT_IN_USE, as on AmigaDOS,
 * where two Locks clash whichever program or task holds them. Like a Lock()
 * after a reset, a flock() is gone when the process holding it ends, however it
 * ends. Open(MODE_READWRITE) opens or makes the file and holds a shared lock
 * while it is open; DeleteFile() refuses an object someone has locked and a
 * drawer that isn't empty. IoErr() is per thread, as it is per process there
 * (libpthread's threads are processes). Forbid()/Permit() is one mutex.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OPENAMIGASQLITE_HOSTDOS_H
#define OPENAMIGASQLITE_HOSTDOS_H
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <unistd.h>

typedef long BPTR;
typedef long LONG;
typedef char *STRPTR;
struct FileInfoBlock { LONG fib_DirEntryType; };

#define SHARED_LOCK             (-2)
#define EXCLUSIVE_LOCK          (-1)
#define MODE_READWRITE          1004
#define DOS_FIB                 2
#define ST_USERDIR              2
#define ST_FILE                 (-3)
#define ERROR_OBJECT_IN_USE     202
#define ERROR_OBJECT_NOT_FOUND  205
#define ERROR_OBJECT_WRONG_TYPE 212
#define ERROR_DIRECTORY_NOT_EMPTY 216

static __thread LONG hostdos_ioerr;
static pthread_mutex_t hostdos_forbid = PTHREAD_MUTEX_INITIALIZER;

static LONG hostdos_err(int e)
{
    hostdos_ioerr = e == EWOULDBLOCK ? ERROR_OBJECT_IN_USE
                  : e == ENOENT ? ERROR_OBJECT_NOT_FOUND
                  : e == EISDIR ? ERROR_OBJECT_WRONG_TYPE
                  : e == ENOTEMPTY ? ERROR_DIRECTORY_NOT_EMPTY : 1000 + e;
    return 0;
}

static LONG IoErr(void) { return hostdos_ioerr; }
static void Forbid(void) { pthread_mutex_lock(&hostdos_forbid); }
static void Permit(void) { pthread_mutex_unlock(&hostdos_forbid); }

/* fd + 1, with flock(how) on it; 0 and IoErr() on failure */
static BPTR hostdos_take(int fd, int how)
{
    int e;
    if (fd < 0) return hostdos_err(errno);
    if (flock(fd, how | LOCK_NB) < 0) { e = errno; close(fd); return hostdos_err(e); }
    return fd + 1;
}

static BPTR Lock(STRPTR name, LONG mode)
{
    return hostdos_take(open(name, O_RDONLY), mode == EXCLUSIVE_LOCK ? LOCK_EX : LOCK_SH);
}
static void UnLock(BPTR l) { if (l) close((int)l - 1); }

static BPTR Open(STRPTR name, LONG mode)
{
    (void)mode;                                         /* MODE_READWRITE only */
    return hostdos_take(open(name, O_RDWR | O_CREAT, 0644), LOCK_SH);
}
static LONG Close(BPTR fh) { close((int)fh - 1); return 1; }

static LONG DeleteFile(STRPTR name)
{
    struct stat st;
    BPTR l = Lock(name, EXCLUSIVE_LOCK);                /* not while someone holds it */
    int r;
    if (!l) return 0;
    r = fstat((int)l - 1, &st) == 0 && (S_ISDIR(st.st_mode) ? rmdir(name) : unlink(name)) == 0;
    if (!r) hostdos_err(errno);
    UnLock(l);
    return r;
}

static void *AllocDosObject(LONG type, void *tags)
{
    (void)type; (void)tags;
    return calloc(1, sizeof(struct FileInfoBlock));
}
static void FreeDosObject(LONG type, void *p) { (void)type; free(p); }

static LONG Examine(BPTR l, struct FileInfoBlock *fib)
{
    struct stat st;
    if (fstat((int)l - 1, &st) < 0) return hostdos_err(errno);
    fib->fib_DirEntryType = S_ISDIR(st.st_mode) ? ST_USERDIR : ST_FILE;
    return 1;
}
#endif
