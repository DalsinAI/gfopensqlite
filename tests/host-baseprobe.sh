#!/bin/sh
# gfopensqlite: tests/baseprobe.c (GFOpenBase's B0 probe) on this computer,
# against the pinned amalgamation with build.sh's options for GFOpenBase
# (THREADSAFE=0) and both Amiga patches, seeking as AmigaDOS does
# (tests/hostseek.h) and locking with the Amiga lock (tests/hostdos.h), as
# tests/host-seektest.sh and tests/host-locktest.sh do. Its "real" answers are
# the ones the Amiga builds are compared with.
# MIT, Copyright (c) 2026 AmiCradle Team.
#
#   SQLITE_SRC   the unpacked sqlite-amalgamation-3530400 folder (required)
#   CC           host C compiler (default cc)
#   CFLAGS       extra flags (e.g. -fsanitize=address,undefined -g)
#   ROWS         the benchmark's size (default 10000)
#
# usage: SQLITE_SRC=... tests/host-baseprobe.sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
: "${SQLITE_SRC:?set SQLITE_SRC to the unpacked sqlite-amalgamation-3530400 folder}"
CC=${CC:-cc}
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
cp "$SQLITE_SRC/sqlite3.c" "$SQLITE_SRC/sqlite3.h" "$W/"
(cd "$W" && patch -s -p1 < "$HERE/patches/sqlite-3.53.4-amiga-paths.patch")
(cd "$W" && patch -s -p1 < "$HERE/patches/sqlite-3.53.4-amiga-seek.patch")
OPTS="-DSQLITE_THREADSAFE=0 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_OMIT_WAL -DSQLITE_MAX_MMAP_SIZE=0 \
    -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_OMIT_DEPRECATED -DSQLITE_DQS=0 -DSQLITE_DISABLE_DIRSYNC -DHAVE_USLEEP=1 \
    -DSQLITE_DEFAULT_UNIX_VFS=\"unix-dotfile\" -DSQLITE_EXTRA_AUTOEXT=sqlite_amiga_autoext \
    -DSQLITE_AMIGA_HOST_LOCKS -DSQLITE_AMIGA_HOST_SEEK -I$HERE/tests"
# shellcheck disable=SC2086
"$CC" -O1 ${CFLAGS:-} $OPTS -U__linux__ -include "$HERE/tests/hostseek.h" -I"$W" -c "$W/sqlite3.c" -o "$W/sqlite3.o"
# shellcheck disable=SC2086
"$CC" -O1 ${CFLAGS:-} $OPTS -I"$W" -c "$HERE/compat/sqlite_amiga.c" -o "$W/sqlite_amiga.o"
# shellcheck disable=SC2086
"$CC" -O1 ${CFLAGS:-} -Wall -Wextra -I"$W" "$HERE/tests/baseprobe.c" "$W/sqlite3.o" "$W/sqlite_amiga.o" -lm -o "$W/baseprobe"
mkdir "$W/db"
"$W/baseprobe" "$W/db/" "${ROWS:-10000}"
