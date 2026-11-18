#!/bin/sh
# gfopensqlite: sqlite.library's Amiga layer (library/sqlite_amiga.c) on
# this computer: its two VFSes, the "<database>.lck" lock, the seek past the
# end of a file, the pool memory and the semaphore mutexes, under SQLite
# built exactly as build-library.sh builds it (SQLITE_OS_OTHER and the rest).
# tests/hostamiga.h stands in for exec and dos.library: a Lock() is a
# flock() (refused when held, gone when its process ends, as after a
# reset), and Seek() refuses a place past the end of the file as AmigaDOS
# does on a real disk.
#
# Three programs run over it:
#   tests/locktest.c    two connections on one database: two writers,
#                       interleaved writers, a reader waiting on another
#                       thread, a writer in another process, a process that
#                       dies in the middle of a write, an old lock drawer
#   tests/baseprobe.c   GFOpenBase's probe: fresh databases, the lock and
#                       "unix-none", journal modes, temporary files, VACUUM
#                       INTO, REAL answers
#   tests/libtest.c     SQLiteLibTest through the link library and the
#                       library's side, as tests/host-libtest.sh, here with
#                       the Amiga layer under it
#
#   SQLITE_SRC   the unpacked sqlite-amalgamation-3530400 folder (required)
#   CC           host C compiler (default cc); CFLAGS is added
#   ROWS         baseprobe's benchmark size (default 2000)
#
# usage: SQLITE_SRC=... tests/host-amigatest.sh
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
: "${SQLITE_SRC:?set SQLITE_SRC to the unpacked sqlite-amalgamation-3530400 folder}"
CC=${CC:-cc}
CFLAGS=${CFLAGS:-}
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
# keep in step with build-library.sh's SQFLAGS
OPTS="-DSQLITE_OS_OTHER=1 -DSQLITE_THREADSAFE=1 -DSQLITE_MUTEX_NOOP -DSQLITE_ZERO_MALLOC \
    -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_OMIT_WAL -DSQLITE_MAX_MMAP_SIZE=0 -DSQLITE_DEFAULT_MEMSTATUS=0 \
    -DSQLITE_OMIT_DEPRECATED -DSQLITE_DQS=0 -DSQLITE_OMIT_SHARED_CACHE -DSQLITE_EXTRA_AUTOEXT=sql_amiga_autoext"
W1="-Wall -Wextra -Werror"
INC="-I$SQLITE_SRC -I$HERE/library -I$HERE/library/link -I$HERE/tests -I$HERE/include"
# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS $OPTS -I"$SQLITE_SRC" -c "$SQLITE_SRC/sqlite3.c" -o "$W/sqlite3.o"
# the stand-in header has calls this file does not use
# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS $W1 -Wno-unused-function -DSQL_AMIGA_HOST $INC -c "$HERE/library/sqlite_amiga.c" -o "$W/sqlite_amiga.o"
# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS $W1 $INC -c "$HERE/tests/host_amiga_start.c" -o "$W/start.o"
LAYER="$W/sqlite3.o $W/sqlite_amiga.o $W/start.o"

# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS -Wall -I"$SQLITE_SRC" "$HERE/tests/locktest.c" $LAYER -lpthread -lm -o "$W/locktest"
(cd "$W" && ./locktest "$W/lock.db")

# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS -Wall -Wextra -I"$SQLITE_SRC" "$HERE/tests/baseprobe.c" $LAYER -lpthread -lm -o "$W/baseprobe"
mkdir "$W/db"
(cd "$W" && ./baseprobe "$W/db/" "${ROWS:-2000}")

# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS $W1 $INC -c "$HERE/tests/host_lib.c" -o "$W/host_lib.o"
# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS $W1 $INC -c "$HERE/library/sqlite_own.c" -o "$W/sqlite_own.o"
PROG="-DSQLITE_LINK_HOST -DSQL_AMIGA_HOST -include $HERE/tests/host_rename.h"
for f in library/link/sqlite3_link library/link/sqlite3_link_more tests/libtest; do
    # shellcheck disable=SC2086
    "$CC" -O1 $CFLAGS $W1 $INC $PROG -c "$HERE/$f.c" -o "$W/$(basename "$f").o"
done
# shellcheck disable=SC2086
"$CC" $CFLAGS -o "$W/libtest" "$W/libtest.o" "$W/sqlite3_link.o" "$W/sqlite3_link_more.o" \
    "$W/host_lib.o" "$W/sqlite_own.o" $LAYER -lpthread -lm
(cd "$W" && ./libtest "$W/libtest.db")
