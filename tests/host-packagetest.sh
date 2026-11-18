#!/bin/sh
# gfopensqlite: tools/package-release.sh over stand-in build folders.
#   every file of the layout is in the .zip, and nothing else;
#   ReadMe.txt names the version, SQLite's version, the library's and the commit;
#   SQLITE-BLESSING.txt is the header's own notice;
#   SHA256SUMS matches the archives, and a second run makes the same .zip;
#   a missing file, sqlite3.h that differs between the builds, a VERSION
#   that is not this SQLite's, and a library that names another SQLite
#   each stop it with no package made.
# usage: tests/host-packagetest.sh
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
checks=0; failed=0
check() { checks=$((checks + 1)); if ! eval "$2"; then failed=$((failed + 1)); echo "FAIL: $1"; fi; }

fresh() {
    rm -rf "$W/hard" "$W/soft" "$W/lib" "$W/dest"
    mkdir -p "$W/hard/include" "$W/hard/lib" "$W/soft/include" "$W/soft/lib" "$W/lib/libs" "$W/lib/tests" \
        "$W/lib/include/libraries" "$W/lib/include/inline" "$W/lib/include/proto" "$W/lib/lib" "$W/lib/lib-soft"
    for d in hard soft lib; do
        printf '/*\n** 2001-09-15\n**\n** The author disclaims copyright to this source code.  In place of\n** a legal notice, here is a blessing:\n**\n**    May you do good and not evil.\n*/\n#define SQLITE_VERSION        "3.53.4"\nint after;\n' > "$W/$d/include/sqlite3.h"
    done
    echo ext > "$W/hard/include/sqlite3ext.h"
    echo hard > "$W/hard/lib/libsqlite3.a"; echo soft > "$W/soft/lib/libsqlite3.a"
    printf 'xx\0$VER: sqlite.library 1.0 (18.11.2026) SQLite 3.53.4, GFOpenSQLite\0yy' > "$W/lib/libs/sqlite.library"
    echo test > "$W/lib/tests/SQLiteLibTest"
    echo link > "$W/lib/include/sqlite3_link.h"
    for d in libraries inline proto; do echo "$d" > "$W/lib/include/$d/sqlite.h"; done
    echo linkhard > "$W/lib/lib/libsqlite3_link.a"; echo linksoft > "$W/lib/lib-soft/libsqlite3_link.a"
}
pack() {
    VERSION=$1 COMMIT=abc1234 OUT_HARD=$W/hard OUT_SOFT=$W/soft OUT_LIBRARY=$W/lib DEST=$W/dest NEED_LHA=0 \
        sh "$HERE/tools/package-release.sh" > "$W/log" 2>&1
}
none() { [ ! -e "$W/dest/GFOpenSQLite-$1.zip" ] && [ ! -e "$W/dest/GFOpenSQLite-$1.lha" ]; }

fresh
rc=0; pack 3.53.4-2-rc1 || rc=$?
check "packs" '[ $rc = 0 ] && [ -s "$W/dest/GFOpenSQLite-3.53.4-2-rc1.zip" ]'
unzip -Z1 "$W/dest/GFOpenSQLite-3.53.4-2-rc1.zip" | grep -v '/$' | LC_ALL=C sort > "$W/got"
LC_ALL=C sort > "$W/want" <<'LIST'
GFOpenSQLite/C/SQLiteLibTest
GFOpenSQLite/Docs/CONTRIBUTORS.md
GFOpenSQLite/Docs/LICENSE
GFOpenSQLite/Docs/README.md
GFOpenSQLite/Docs/SOURCES
GFOpenSQLite/Docs/SQLITE-BLESSING.txt
GFOpenSQLite/Docs/sqlite.library.md
GFOpenSQLite/Libs/sqlite.library
GFOpenSQLite/ReadMe.txt
GFOpenSQLite/SDK/Include/inline/sqlite.h
GFOpenSQLite/SDK/Include/libraries/sqlite.h
GFOpenSQLite/SDK/Include/proto/sqlite.h
GFOpenSQLite/SDK/Include/sqlite3.h
GFOpenSQLite/SDK/Include/sqlite3_link.h
GFOpenSQLite/SDK/Include/sqlite3ext.h
GFOpenSQLite/SDK/lib/libsqlite3.a
GFOpenSQLite/SDK/lib/libsqlite3_link.a
GFOpenSQLite/SDK/lib/soft-float/fpuauto.wraps
GFOpenSQLite/SDK/lib/soft-float/libsqlite3.a
GFOpenSQLite/SDK/lib/soft-float/libsqlite3_link.a
GFOpenSQLite/SDK/sfd/sqlite_lib.sfd
LIST
check "the layout, whole and nothing else" 'cmp -s "$W/got" "$W/want"'
mkdir -p "$W/x" && (cd "$W/x" && unzip -qo "$W/dest/GFOpenSQLite-3.53.4-2-rc1.zip")
G=$W/x/GFOpenSQLite
check "each library in its place" '[ "$(cat "$G/SDK/lib/libsqlite3.a")" = hard ] && [ "$(cat "$G/SDK/lib/soft-float/libsqlite3.a")" = soft ] && [ "$(cat "$G/SDK/lib/libsqlite3_link.a")" = linkhard ] && [ "$(cat "$G/SDK/lib/soft-float/libsqlite3_link.a")" = linksoft ]'
check "the library itself, byte for byte" 'cmp -s "$G/Libs/sqlite.library" "$W/lib/libs/sqlite.library"'
check "the sfd is the repository's" 'cmp -s "$G/SDK/sfd/sqlite_lib.sfd" "$HERE/library/sqlite_lib.sfd"'
check "ReadMe: version, SQLite, library, commit" 'grep -q "GFOpenSQLite 3.53.4-2-rc1: SQLite 3.53.4 " "$G/ReadMe.txt" && grep -q "sqlite.library 1.0 (18.11.2026)" "$G/ReadMe.txt" && grep -q "Built from commit abc1234" "$G/ReadMe.txt"'
check "the notice is the header's, and ends where the comment does" 'grep -q "May you do good and not evil" "$G/Docs/SQLITE-BLESSING.txt" && ! grep -q "SQLITE_VERSION\|after" "$G/Docs/SQLITE-BLESSING.txt"'
check "SHA256SUMS matches" '(cd "$W/dest" && sha256sum -c SHA256SUMS >/dev/null 2>&1)'
cp "$W/dest/GFOpenSQLite-3.53.4-2-rc1.zip" "$W/first.zip"
rc=0; pack 3.53.4-2-rc1 || rc=$?
check "a second run makes the same .zip" '[ $rc = 0 ] && cmp -s "$W/first.zip" "$W/dest/GFOpenSQLite-3.53.4-2-rc1.zip"'

fresh; rm "$W/lib/lib-soft/libsqlite3_link.a"
rc=0; pack 3.53.4-2 || rc=$?
check "a missing file stops it" '[ $rc != 0 ] && grep -q "is not there" "$W/log" && none 3.53.4-2'
fresh; echo "int more;" >> "$W/soft/include/sqlite3.h"
rc=0; pack 3.53.4-2 || rc=$?
check "two SQLites stop it" '[ $rc != 0 ] && grep -q "not one SQLite" "$W/log" && none 3.53.4-2'
fresh
rc=0; pack 3.54.0-1 || rc=$?
check "a VERSION of another SQLite stops it" '[ $rc != 0 ] && grep -q "does not begin" "$W/log" && none 3.54.0-1'
fresh; printf '$VER: sqlite.library 1.0 (18.11.2026) SQLite 3.50.0, GFOpenSQLite' > "$W/lib/libs/sqlite.library"
rc=0; pack 3.53.4-2 || rc=$?
check "a library holding another SQLite stops it" '[ $rc != 0 ] && grep -q "does not name SQLite" "$W/log" && none 3.53.4-2'
fresh
rc=0; pack "3.53.4-2 x" || rc=$?
check "a VERSION with a space stops it" '[ $rc != 0 ]'

if [ $failed = 0 ]; then echo "PACKAGETEST PASS: $checks checks"; else echo "PACKAGETEST FAIL: $failed of $checks"; exit 1; fi
