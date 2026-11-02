# gfopensqlite

SQLite for AmigaOS 3.x on 68k, built as static link libraries for
GCC programs. Part of the [GFOpenAmiga](https://github.com/DalsinAI/gfopenamiga)
ports, made for [GFOpenBrowser](https://github.com/DalsinAI/gfopenbrowser),
the WebKit browser for AmigaOS 3.2.

**Status:** Working: builds, and the smoke test passes on the bench with the database on DH1:.

This repository holds the Amiga build, not SQLite itself: a build script,
patches, a smoke test and the upstream licences.

## Upstream

| Library | Version | Licence | Home |
| --- | --- | --- | --- |
| SQLite | 3.53.4 | public domain (see upstream/SQLITE-BLESSING.txt) | https://www.sqlite.org/ |

The exact files and their SHA-256 sums are in [SOURCES](SOURCES). All credit
for the library goes to its authors; see `upstream/` for their notices.

## What the Amiga port changes

- **AmigaDOS names.** The unix VFS turns every file name into a POSIX path, which breaks names like `T:a.db` (and a leading `/` means the parent directory on AmigaDOS). The patch keeps a name with a volume or assign as given, and makes a relative one (`Cookies.db`, `/x.db`) whole from its drawer's name (`NameFromLock`, `compat/sqlite_amiga.c`), so the `-journal` stays beside the database when the program changes its current directory (`patches/sqlite-3.53.4-amiga-paths.patch`). Link `compat/sqlite_amiga.c` with it.
- **`fchmod` and `fchown`.** libnix has neither; `compat/sqlite_amiga.c` supplies ones that succeed and do nothing, since AmigaOS files have no Unix owner or mode bits.
- **No WAL, no mmap, no extension loading.** These need shared memory or `dlopen`, which AmigaOS doesn't have.
- **Locking.** libnix has no POSIX advisory locks (its `fcntl()` always fails, so SQLite's own default, "unix", gives "disk I/O error" on the first statement). The default VFS is therefore **unix-dotfile**, with an Amiga lock: an exclusive `Lock()` on the file `<database>.lck` (`compat/sqlite_amiga.c`, through `patches/sqlite-3.53.4-amiga-paths.patch`). When another connection, in the same program or another, holds it, `Lock()` fails with `ERROR_OBJECT_IN_USE` and SQLite treats the database as busy. It is one lock for readers and writers alike, so every connection waits up to 2 s for it (a busy timeout set as each connection opens, `compat/sqlite_amiga.c`; `sqlite3_busy_timeout()` changes it). Programs need do nothing. One that keeps its own lock can still ask for "unix-none" in `sqlite3_open_v2()`, as GFOpenBase does with its semaphore; that takes no lock and makes no `.lck` file.
- **No lock left behind.** Upstream's dotfile lock is a drawer, `<database>.lock`, made with `mkdir` and deleted at unlock, and the first builds here used it: a reset or crash in the middle of a statement left the drawer, and every connection after it waited 2 s and got "database is locked" (GFOpenBrowser's cookie store then deleted Cookies.db). A `Lock()` is gone after a reset, and the library lets go of any its connections still hold when the program calls `exit()` (AmigaDOS itself doesn't free a program's locks when it ends). The `.lck` file is empty and stays beside the database: it is never deleted, as another connection may hold it. An empty `<database>.lock` drawer left by an earlier build is deleted the first time a connection takes the new lock.
- **What can still hold it.** A program that crashes and is not reset (a task left suspended after a software failure) still holds its `Lock()`, so the database stays busy until the reset; it can't be told apart from a program that is still working. Two builds of different ages (drawer and `.lck`) don't see each other's lock, so don't run an old and a new build on one database at the same time.

## Building

You need the os32-gcc16 compiler (bebbo's amiga-gcc on GCC 16.2 with libnix
and libpthread; see DalsinAI/gfopenbrowser `stove/`) and the upstream
tarballs from [SOURCES](SOURCES) in `tarballs/`. Then:

```
./build.sh
```

The libraries and headers land in `out/` (set `PREFIX` to change that). The
script prints which other settings it needs, if any. Target: the 68040
with its FPU (`-m68040 -m68881`), libnix (`-mcrt=nix20`).

It is hard float, so it needs a 68040 with its FPU (not a 68LC040 or
68EC040). It is part of the browser stack, which stays hard float for now.

Link with: `-lsqlite3 -lpthread`

## Tested

`tests/locktest.c` (two connections on one database: two writers, interleaved writers, a reader during a write; on the host also a writer in another process and a process that dies in the middle of a write transaction; an earlier build's lock drawer) passes on this computer with `SQLITE_SRC=... tests/host-locktest.sh`, against the pinned amalgamation built with `build.sh`'s options and libnix's `fcntl()` copied. There the Amiga lock code runs with `tests/hostdos.h` standing in for dos.library: a `Lock()` is a `flock()`, which, like a `Lock()` after a reset, is gone when the process holding it ends. It builds for AmigaOS too (`-lsqlite3 -lpthread`; run it as `locktest DH1:Probe/lock.db`; the two tests that need a second process are host only).

The smoke test (this run came before unix-dotfile was the default, when the test registered unix-none; it now prints `vfs=unix-dotfile`):

`tests/sqlitetest.c`, run on AmigaOS 3.2.3 on AmiCradle's AC090 emulation (68040 with FPU, 256 MB), Instance-24, 4 October 2026, as `sqlitetest DH1:Probe/sqlt.db keep`:

```
SQLITE 3.53.4 vfs=unix-none
id=3 name=cookie v=-1.5 len=6
id=1 name=amiga v=3.2 len=5
id=2 name=webkit v=605.1 len=6
n=3 total=606.80000000000007 u=OK
SQLITE_DONE rc=0
```

The database file it wrote passes `PRAGMA integrity_check` in SQLite on Linux.

It has not yet been run on real Amiga hardware.

## Known issues

- In `RAM:` (`T:`), a fresh database gives "file is not a database" on the first statement. On DH1: it works. Not yet investigated, so keep databases on disk for now.

## Licence

AmiCradle Team's Amiga changes (the build script, patches, configuration
headers and tests) are MIT, Copyright (c) 2026 AmiCradle Team: see
[LICENSE](LICENSE). SQLite keeps its own licence, in
[upstream/](upstream/); a patch to its source stays under that licence.

## Contributors

This port is maintained by [SacredTrees](https://github.com/SacredTrees) with The AmiCradle Team, copyright AmiCradle Team. Everyone whose work it includes is credited in [`CONTRIBUTORS.md`](CONTRIBUTORS.md).
