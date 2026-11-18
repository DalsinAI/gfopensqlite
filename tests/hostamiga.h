/* gfopensqlite: the exec and dos.library calls library/sqlite_amiga.c makes,
 * stood in for on this computer, so sqlite.library's file layer, memory
 * and mutexes run under tests/host-amigatest.sh (SQL_AMIGA_HOST).
 *
 * Files and locks are tests/hostdos.h's, with the rest of what a VFS needs:
 *   - A Lock() is an open descriptor with flock() on it; a second exclusive
 *     one is refused with ERROR_OBJECT_IN_USE whichever thread or process
 *     asks, and it is gone when its process ends, as after a reset.
 *   - Open() holds a shared lock on the file while it is open.
 *   - Seek() refuses a place past the end of the file, as AmigaDOS does on
 *     a real disk; SetFileSize() grows or cuts the file.
 *   - A SignalSemaphore is a recursive mutex; a pool is a list of blocks,
 *     all freed by DeletePool().
 *   - There is no locale.library: the clock is taken as UTC.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef GFOPENSQLITE_HOSTAMIGA_H
#define GFOPENSQLITE_HOSTAMIGA_H
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

typedef long BPTR;
typedef int LONG;
typedef unsigned int ULONG;
typedef unsigned char UBYTE;
typedef void *APTR;
typedef char *STRPTR;
typedef const char *CONST_STRPTR;
struct FileInfoBlock { LONG fib_DirEntryType; };
struct DateStamp { LONG ds_Days, ds_Minute, ds_Tick; };
struct Library { int unused; };
struct Task { int unused; };

#define SHARED_LOCK             (-2)
#define ACCESS_READ             (-2)
#define EXCLUSIVE_LOCK          (-1)
#define MODE_OLDFILE            1005
#define MODE_NEWFILE            1006
#define MODE_READWRITE          1004
#define OFFSET_BEGINNING        (-1)
#define OFFSET_CURRENT          0
#define OFFSET_END              1
#define DOS_FIB                 2
#define ST_USERDIR              2
#define ST_FILE                 (-3)
#define ERROR_OBJECT_IN_USE     202
#define ERROR_OBJECT_NOT_FOUND  205
#define ERROR_SEEK_ERROR        219
#define ERROR_OBJECT_WRONG_TYPE 212
#define ERROR_DIRECTORY_NOT_EMPTY 216
#define ERROR_DISK_FULL         221
#define MEMF_ANY                0
#define MEMF_CLEAR              (1 << 16)

/* ---- exec ----------------------------------------------------------------- */

struct SignalSemaphore { pthread_mutex_t m; };

static void InitSemaphore(struct SignalSemaphore *s)
{
    pthread_mutexattr_t a;
    pthread_mutexattr_init(&a);
    pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&s->m, &a);
    pthread_mutexattr_destroy(&a);
}
static void ObtainSemaphore(struct SignalSemaphore *s) { pthread_mutex_lock(&s->m); }
static void ReleaseSemaphore(struct SignalSemaphore *s) { pthread_mutex_unlock(&s->m); }
static ULONG AttemptSemaphore(struct SignalSemaphore *s) { return pthread_mutex_trylock(&s->m) == 0; }

static pthread_mutex_t hostamiga_forbid = PTHREAD_MUTEX_INITIALIZER;
static void Forbid(void) { pthread_mutex_lock(&hostamiga_forbid); }
static void Permit(void) { pthread_mutex_unlock(&hostamiga_forbid); }

static struct Task *FindTask(const char *name) { (void)name; return (struct Task *)(size_t)pthread_self(); }
static void CopyMem(const void *s, void *d, ULONG n) { memcpy(d, s, n); }

static APTR AllocMem(ULONG n, ULONG flags) { return flags & MEMF_CLEAR ? calloc(1, n) : malloc(n); }
static void FreeMem(APTR p, ULONG n) { (void)n; free(p); }

/* a pool: every block on a list, so DeletePool frees what was never freed */
struct hostamiga_block { struct hostamiga_block *next, *prev; };
struct hostamiga_pool { struct hostamiga_block head; };

static APTR CreatePool(ULONG flags, ULONG puddle, ULONG thresh)
{
    struct hostamiga_pool *p = (struct hostamiga_pool *)malloc(sizeof *p);
    (void)flags; (void)puddle; (void)thresh;
    if (p) p->head.next = p->head.prev = &p->head;
    return p;
}

static APTR AllocPooled(APTR pool, ULONG n)
{
    struct hostamiga_pool *p = (struct hostamiga_pool *)pool;
    struct hostamiga_block *b = (struct hostamiga_block *)malloc(sizeof *b + n);
    if (!b) return 0;
    b->next = p->head.next; b->prev = &p->head;
    b->next->prev = b; p->head.next = b;
    return b + 1;
}

static void FreePooled(APTR pool, APTR mem, ULONG n)
{
    struct hostamiga_block *b = (struct hostamiga_block *)mem - 1;
    (void)pool; (void)n;
    b->prev->next = b->next; b->next->prev = b->prev;
    free(b);
}

static void DeletePool(APTR pool)
{
    struct hostamiga_pool *p = (struct hostamiga_pool *)pool;
    while (p->head.next != &p->head) FreePooled(p, p->head.next + 1, 0);
    free(p);
}

/* ---- dos.library ---------------------------------------------------------- */

static __thread LONG hostamiga_ioerr;

static LONG hostamiga_err(int e)
{
    hostamiga_ioerr = e == EWOULDBLOCK ? ERROR_OBJECT_IN_USE
                    : e == ENOENT ? ERROR_OBJECT_NOT_FOUND
                    : e == EISDIR ? ERROR_OBJECT_WRONG_TYPE
                    : e == ENOSPC ? ERROR_DISK_FULL
                    : e == ENOTEMPTY ? ERROR_DIRECTORY_NOT_EMPTY : 1000 + e;
    return 0;
}

static LONG IoErr(void) { return hostamiga_ioerr; }

/* fd + 1, with flock(how) on it; 0 and IoErr() on failure */
static BPTR hostamiga_take(int fd, int how)
{
    int e;
    if (fd < 0) return hostamiga_err(errno);
    if (flock(fd, how | LOCK_NB) < 0) { e = errno; close(fd); return hostamiga_err(e); }
    return fd + 1;
}

static BPTR Lock(STRPTR name, LONG mode)
{
    return hostamiga_take(open(*name ? name : ".", O_RDONLY), mode == EXCLUSIVE_LOCK ? LOCK_EX : LOCK_SH);
}
static void UnLock(BPTR l) { if (l) close((int)l - 1); }

static BPTR Open(STRPTR name, LONG mode)
{
    if (mode == MODE_OLDFILE) return hostamiga_take(open(name, O_RDWR), LOCK_SH);
    if (mode == MODE_NEWFILE) return hostamiga_take(open(name, O_RDWR | O_CREAT | O_TRUNC, 0644), LOCK_EX);
    return hostamiga_take(open(name, O_RDWR | O_CREAT, 0644), LOCK_SH);
}
static LONG Close(BPTR fh) { close((int)fh - 1); return 1; }

static LONG Read(BPTR fh, APTR buf, LONG n)
{
    ssize_t got = read((int)fh - 1, buf, (size_t)n);
    if (got < 0) { hostamiga_err(errno); return -1; }
    return (LONG)got;
}

static LONG Write(BPTR fh, APTR buf, LONG n)
{
    ssize_t put = write((int)fh - 1, buf, (size_t)n);
    if (put < 0) { hostamiga_err(errno); return -1; }
    return (LONG)put;
}

/* the old position, or -1: past the end of the file is refused */
static LONG Seek(BPTR fh, LONG off, LONG mode)
{
    int fd = (int)fh - 1;
    struct stat st;
    off_t old = lseek(fd, 0, SEEK_CUR), to;
    if (old < 0 || fstat(fd, &st) < 0) { hostamiga_err(errno); return -1; }
    to = mode == OFFSET_BEGINNING ? off : mode == OFFSET_END ? st.st_size + off : old + off;
    if (to < 0 || to > st.st_size) { hostamiga_ioerr = ERROR_SEEK_ERROR; return -1; }
    lseek(fd, to, SEEK_SET);
    return (LONG)old;
}

static LONG SetFileSize(BPTR fh, LONG off, LONG mode)
{
    (void)mode;                                         /* OFFSET_BEGINNING only */
    if (ftruncate((int)fh - 1, off) < 0) { hostamiga_err(errno); return -1; }
    return off;
}

static LONG DeleteFile(STRPTR name)
{
    struct stat st;
    BPTR l = Lock(name, EXCLUSIVE_LOCK);                /* not while someone holds it */
    int r;
    if (!l) return 0;
    r = fstat((int)l - 1, &st) == 0 && (S_ISDIR(st.st_mode) ? rmdir(name) : unlink(name)) == 0;
    if (!r) hostamiga_err(errno);
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
    if (fstat((int)l - 1, &st) < 0) return hostamiga_err(errno);
    fib->fib_DirEntryType = S_ISDIR(st.st_mode) ? ST_USERDIR : ST_FILE;
    return 1;
}

static LONG NameFromLock(BPTR l, STRPTR buf, LONG n)
{
    char link[64];
    ssize_t k;
    snprintf(link, sizeof link, "/proc/self/fd/%d", (int)l - 1);
    k = readlink(link, buf, (size_t)n - 1);
    if (k < 0) return hostamiga_err(errno);
    buf[k] = 0;
    return 1;
}

/* the file's name within a path, and where the path's drawer part ends */
static STRPTR FilePart(STRPTR path)
{
    char *s = strrchr(path, '/');
    return s ? s + 1 : path;
}
static STRPTR PathPart(STRPTR path)
{
    char *s = strrchr(path, '/');
    return s ? (s == path ? s + 1 : s) : path;
}
static LONG AddPart(STRPTR dir, STRPTR file, ULONG n)
{
    size_t d = strlen(dir), f = strlen(file);
    int slash = d && dir[d - 1] != '/';
    if (d + slash + f + 1 > n) return 0;
    if (slash) dir[d++] = '/';
    memcpy(dir + d, file, f + 1);
    return 1;
}

/* days, minutes and 50ths of a second since 1 January 1978 */
static struct DateStamp *DateStamp(struct DateStamp *ds)
{
    struct timeval tv;
    long s;
    gettimeofday(&tv, 0);
    s = (long)tv.tv_sec - 252460800L;
    ds->ds_Days = (LONG)(s / 86400);
    ds->ds_Minute = (LONG)(s % 86400 / 60);
    ds->ds_Tick = (LONG)(s % 60 * 50 + tv.tv_usec / 20000);
    return ds;
}

static void Delay(LONG ticks) { usleep((useconds_t)ticks * 20000); }

#endif
