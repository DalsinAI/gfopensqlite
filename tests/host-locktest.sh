#!/bin/sh
# gfopensqlite: tests/locktest.c on this computer, against the pinned
# amalgamation built with build.sh's SQLite options (keep the two in step).
# libnix's fcntl() is copied with -libnix-fcntl, so the VFSes meet what they
# meet on AmigaOS. The unix-dotfile lock is the Amiga one, compat/sqlite_amiga.c's
# exclusive Lock() on "<database>.lck", with tests/hostdos.h standing in for
# dos.library (SQLITE_AMIGA_HOST_LOCKS): a Lock() is a flock(), gone when the
# process holding it ends, as a Lock() is after a reset.
# MIT, Copyright (c) 2026 AmiCradle Team.
#
#   SQLITE_SRC   the unpacked sqlite-amalgamation-3530400 folder (required)
#   CC           host C compiler (default cc)
#
# usage: SQLITE_SRC=... tests/host-locktest.sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
: "${SQLITE_SRC:?set SQLITE_SRC to the unpacked sqlite-amalgamation-3530400 folder}"
CC=${CC:-cc}
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
cp "$SQLITE_SRC/sqlite3.c" "$SQLITE_SRC/sqlite3.h" "$W/"
(cd "$W" && patch -s -p1 < "$HERE/patches/sqlite-3.53.4-amiga-paths.patch")
OPTS="-DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_OMIT_WAL -DSQLITE_MAX_MMAP_SIZE=0 \
    -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_OMIT_DEPRECATED -DSQLITE_DQS=0 -DHAVE_USLEEP=1 \
    -DSQLITE_DEFAULT_UNIX_VFS=\"unix-dotfile\" -DSQLITE_EXTRA_AUTOEXT=sqlite_amiga_autoext \
    -DSQLITE_AMIGA_HOST_LOCKS -I$HERE/tests"
# shellcheck disable=SC2086
"$CC" -O1 $OPTS -I"$W" -c "$W/sqlite3.c" -o "$W/sqlite3.o"
# shellcheck disable=SC2086
"$CC" -O1 $OPTS -I"$W" -c "$HERE/compat/sqlite_amiga.c" -o "$W/sqlite_amiga.o"
"$CC" -O1 -Wall -I"$W" "$HERE/tests/locktest.c" "$W/sqlite3.o" "$W/sqlite_amiga.o" -lpthread -lm -o "$W/locktest"
"$W/locktest" -libnix-fcntl "$W/lock.db"
