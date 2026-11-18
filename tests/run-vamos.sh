#!/bin/sh
# gfopensqlite: SQLiteLibTest through the real sqlite.library, as 68040
# code, under vamos (amitools) on this computer. Run after build-library.sh.
#
#   OUT    build-library.sh's output folder (default ./out-library)
#   VAMOS  the vamos command (default vamos)
#
# vamos runs one task, so the test's second-task part is left out (its
# "notasks" argument). vamos does not refuse a second exclusive Lock() on a
# file, which the database lock is built on, so the test leaves out the
# checks that one connection keeps another out (it finds that out itself).
# vamos has no FPU, so this is the library's soft-float path. Those three
# (a second task, the lock, the FPU path through fpuauto) need an Amiga.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
OUT=$(CDPATH= cd -- "${OUT:-"$HERE/out-library"}" && pwd)
VAMOS=${VAMOS:-vamos}
V="$OUT/vamos"
rm -rf "$V"; mkdir -p "$V/libs" "$V/t"
cp "$OUT/libs/sqlite.library" "$V/libs/"
cp "$OUT/tests/SQLiteLibTestSoft" "$V/"
cd "$V"
# twice: the first run ends with a connection open and a write begun, which
# the link library's exit closes (each vamos is a new machine, so the
# second run shows only that the exit did not crash and left a good file)
for n in 1 2; do
    "$VAMOS" -C 68040 -m 65536 -s 16 -H disable -V "vam:$V" -a LIBS:vam:libs -a T:vam:t \
        SQLiteLibTestSoft vam:t/libtest.db notasks
done
