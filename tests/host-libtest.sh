#!/bin/sh
# gfopensqlite: tests/libtest.c (SQLiteLibTest) on this computer.
#
# The program calls sqlite3_open() and the rest. Here those names are the
# link library's functions (library/link/, under lnk_ names:
# tests/host_rename.h), which call sqlite.library's side
# (library/sqlite_lib_calls.h and sqlite_own.c, tests/host_lib.c) as plain C
# calls (tests/host_inline.h), which calls the real SQLite, built with
# build-library.sh's options apart from the Amiga's files, memory and
# mutexes. So every 64-bit split, every double by pointer, every callback
# cast, every unpacked argument list and the owner tokens run as they are
# written; what it can't show is the registers themselves and the Amiga
# layer (library/sqlite_amiga.c), which need an Amiga.
#
#   SQLITE_SRC   the unpacked sqlite-amalgamation-3530400 folder (required)
#   CC           host C compiler (default cc); CFLAGS is added
#                (-fsanitize=address,undefined for the sanitizers)
#
# usage: SQLITE_SRC=... tests/host-libtest.sh
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
: "${SQLITE_SRC:?set SQLITE_SRC to the unpacked sqlite-amalgamation-3530400 folder}"
CC=${CC:-cc}
CFLAGS=${CFLAGS:-}
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
python3 "$HERE/tools/gen-library.py" --check
OPTS="-DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_OMIT_WAL -DSQLITE_MAX_MMAP_SIZE=0 \
    -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_OMIT_DEPRECATED -DSQLITE_DQS=0 -DSQLITE_OMIT_SHARED_CACHE"
W1="-Wall -Wextra -Werror"
INC="-I$SQLITE_SRC -I$HERE/library -I$HERE/library/link -I$HERE/tests -I$HERE/include"
# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS $OPTS -I"$SQLITE_SRC" -c "$SQLITE_SRC/sqlite3.c" -o "$W/sqlite3.o"
# the library's side: the real sqlite3_ names
# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS $W1 $INC -c "$HERE/tests/host_lib.c" -o "$W/host_lib.o"
# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS $W1 $INC -c "$HERE/library/sqlite_own.c" -o "$W/sqlite_own.o"
# the program's side: the link library and the test, their sqlite3_ names renamed
PROG="-DSQLITE_LINK_HOST -include $HERE/tests/host_rename.h"
# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS $W1 $INC $PROG -c "$HERE/library/link/sqlite3_link.c" -o "$W/sqlite3_link.o"
# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS $W1 $INC $PROG -c "$HERE/library/link/sqlite3_link_more.c" -o "$W/sqlite3_link_more.o"
# shellcheck disable=SC2086
"$CC" -O1 $CFLAGS $W1 $INC $PROG -c "$HERE/tests/libtest.c" -o "$W/libtest.o"
# the program's objects call nothing of SQLite's but through the link library
if nm -u "$W/libtest.o" "$W/sqlite3_link.o" "$W/sqlite3_link_more.o" | grep ' sqlite3_'; then
    echo "host-libtest: a call went round the link library"; exit 1
fi
# shellcheck disable=SC2086
"$CC" $CFLAGS -o "$W/libtest" "$W/libtest.o" "$W/sqlite3_link.o" "$W/sqlite3_link_more.o" \
    "$W/host_lib.o" "$W/sqlite_own.o" "$W/sqlite3.o" -lpthread -lm
"$W/libtest" "$W/libtest.db"
