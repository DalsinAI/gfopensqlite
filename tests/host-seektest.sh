#!/bin/sh
# gfopensqlite: tests/seektest.c on this computer, against the pinned
# amalgamation with build.sh's patches and SQLite options (keep the two in
# step). lseek() refuses a position past the end of a file as AmigaDOS's
# Seek() does on a real disk (tests/hostseek.h), and the seek patch is
# switched on with SQLITE_AMIGA_HOST_SEEK. With NOSEEKPATCH=1 the seek
# patch is left out, to see the test fail without it.
# MIT, Copyright (c) 2026 AmiCradle Team.
#
#   SQLITE_SRC   the unpacked sqlite-amalgamation-3530400 folder (required)
#   CC           host C compiler (default cc)
#
# usage: SQLITE_SRC=... tests/host-seektest.sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
: "${SQLITE_SRC:?set SQLITE_SRC to the unpacked sqlite-amalgamation-3530400 folder}"
CC=${CC:-cc}
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
cp "$SQLITE_SRC/sqlite3.c" "$SQLITE_SRC/sqlite3.h" "$W/"
(cd "$W" && patch -s -p1 < "$HERE/patches/sqlite-3.53.4-amiga-paths.patch")
[ -n "${NOSEEKPATCH:-}" ] || (cd "$W" && patch -s -p1 < "$HERE/patches/sqlite-3.53.4-amiga-seek.patch")
OPTS="-DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_OMIT_WAL -DSQLITE_MAX_MMAP_SIZE=0 \
    -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_OMIT_DEPRECATED -DSQLITE_DQS=0 -DHAVE_USLEEP=1 \
    -DSQLITE_DEFAULT_UNIX_VFS=\"unix-dotfile\" -DSQLITE_EXTRA_AUTOEXT=sqlite_amiga_autoext \
    -DSQLITE_AMIGA_HOST_LOCKS -DSQLITE_AMIGA_HOST_SEEK -I$HERE/tests"
# -U__linux__: on Linux SQLite reads and writes with pread()/pwrite() and
# never seeks; libnix has neither, so the Amiga build seeks
# shellcheck disable=SC2086
"$CC" -O1 $OPTS -U__linux__ -include "$HERE/tests/hostseek.h" -I"$W" -c "$W/sqlite3.c" -o "$W/sqlite3.o"
# shellcheck disable=SC2086
"$CC" -O1 $OPTS -I"$W" -c "$HERE/compat/sqlite_amiga.c" -o "$W/sqlite_amiga.o"
"$CC" -O1 -Wall -I"$W" "$HERE/tests/seektest.c" "$W/sqlite3.o" "$W/sqlite_amiga.o" -lpthread -lm -o "$W/seektest"
"$W/seektest" "$W"
