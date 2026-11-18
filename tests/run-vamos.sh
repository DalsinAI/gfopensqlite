#!/bin/sh
# gfopensqlite: SQLiteLibTest through the real sqlite.library, as 68040
# code, under vamos (amitools) on this computer. Run after build-library.sh.
#
#   OUT    build-library.sh's output folder (default ./out-library)
#   VAMOS  the vamos command (default vamos)
#
# vamos runs one task, so the test's second-task part is left out (its
# "notasks" argument); that part needs an Amiga. vamos has no FPU either, so
# this is the library's soft-float path; the FPU path (fpuauto) needs an
# Amiga too. vamos never frees a lock's key slot and allows 1024, which one
# run of the test stays under.
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
# the link library's exit must close for the second to pass
for n in 1 2; do
    "$VAMOS" -C 68040 -m 65536 -s 16 -H disable -V "vam:$V" -a LIBS:vam:libs -a T:vam:t \
        SQLiteLibTestSoft vam:t/libtest.db notasks
done
