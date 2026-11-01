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
