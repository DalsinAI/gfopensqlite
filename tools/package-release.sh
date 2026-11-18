#!/bin/sh
# gfopensqlite: the release package, GFOpenSQLite-<VERSION>.lha and .zip.
#
# Takes what build.sh and build-library.sh made and lays it out as the
# release has always been laid out, with sqlite.library and its link
# library added:
#
#   GFOpenSQLite/ReadMe.txt
#   GFOpenSQLite/Libs/sqlite.library                       copy to LIBS:
#   GFOpenSQLite/C/SQLiteLibTest                           checks an install
#   GFOpenSQLite/SDK/Include/sqlite3.h sqlite3ext.h sqlite3_link.h
#   GFOpenSQLite/SDK/Include/libraries/ inline/ proto/     sqlite.h
#   GFOpenSQLite/SDK/sfd/sqlite_lib.sfd
#   GFOpenSQLite/SDK/lib/libsqlite3_link.a                 on the library, FPU
#   GFOpenSQLite/SDK/lib/libsqlite3.a                      static, FPU
#   GFOpenSQLite/SDK/lib/soft-float/libsqlite3_link.a      on the library, no FPU
#   GFOpenSQLite/SDK/lib/soft-float/libsqlite3.a           static, no FPU
#   GFOpenSQLite/SDK/lib/soft-float/fpuauto.wraps
#   GFOpenSQLite/Docs/                                     README.md, sqlite.library.md,
#                                                          LICENSE, CONTRIBUTORS.md, SOURCES,
#                                                          SQLITE-BLESSING.txt
#
#   VERSION      the release's name, as 3.53.4-2 or 3.53.4-2-rc1 (required)
#   COMMIT       the commit the files were built from (default: git's HEAD)
#   OUT_HARD     build.sh's folder (default ./out)
#   OUT_SOFT     FLOAT=soft build.sh's folder (default ./out-soft)
#   OUT_LIBRARY  build-library.sh's folder (default ./out-library)
#   DEST         where the package goes (default ./release)
#
# A file that is missing stops it: no package is made with a part left out.
# usage: VERSION=3.53.4-2-rc1 tools/package-release.sh
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
: "${VERSION:?set VERSION, as 3.53.4-2 or 3.53.4-2-rc1}"
case "$VERSION" in
    *[!0-9A-Za-z.-]*) echo "package-release: VERSION may hold letters, digits, '.' and '-' only"; exit 2 ;;
esac
COMMIT=${COMMIT:-$(git -C "$HERE" rev-parse --short=7 HEAD)}
OUT_HARD=${OUT_HARD:-"$HERE/out"}
OUT_SOFT=${OUT_SOFT:-"$HERE/out-soft"}
OUT_LIBRARY=${OUT_LIBRARY:-"$HERE/out-library"}
DEST=${DEST:-"$HERE/release"}
NAME=GFOpenSQLite-$VERSION
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
R=$W/GFOpenSQLite

# put FROM TO: one file into the package, or stop
put() {
    [ -f "$1" ] || { echo "package-release: $1 is not there"; exit 1; }
    mkdir -p "$(dirname -- "$R/$2")"
    cp "$1" "$R/$2"
}

put "$OUT_LIBRARY/libs/sqlite.library"            Libs/sqlite.library
put "$OUT_LIBRARY/tests/SQLiteLibTest"            C/SQLiteLibTest
put "$OUT_LIBRARY/include/sqlite3.h"              SDK/Include/sqlite3.h
put "$OUT_HARD/include/sqlite3ext.h"              SDK/Include/sqlite3ext.h
put "$OUT_LIBRARY/include/sqlite3_link.h"         SDK/Include/sqlite3_link.h
put "$OUT_LIBRARY/include/libraries/sqlite.h"     SDK/Include/libraries/sqlite.h
put "$OUT_LIBRARY/include/inline/sqlite.h"        SDK/Include/inline/sqlite.h
put "$OUT_LIBRARY/include/proto/sqlite.h"         SDK/Include/proto/sqlite.h
put "$HERE/library/sqlite_lib.sfd"                SDK/sfd/sqlite_lib.sfd
put "$OUT_LIBRARY/lib/libsqlite3_link.a"          SDK/lib/libsqlite3_link.a
put "$OUT_HARD/lib/libsqlite3.a"                  SDK/lib/libsqlite3.a
put "$OUT_LIBRARY/lib-soft/libsqlite3_link.a"     SDK/lib/soft-float/libsqlite3_link.a
put "$OUT_SOFT/lib/libsqlite3.a"                  SDK/lib/soft-float/libsqlite3.a
put "$HERE/fpuauto/fpuauto.wraps"                 SDK/lib/soft-float/fpuauto.wraps
put "$HERE/README.md"                             Docs/README.md
put "$HERE/docs/sqlite.library.md"                Docs/sqlite.library.md
put "$HERE/LICENSE"                               Docs/LICENSE
put "$HERE/CONTRIBUTORS.md"                       Docs/CONTRIBUTORS.md
put "$HERE/SOURCES"                               Docs/SOURCES

# the two sqlite3.h must be the same file: one SQLite in the whole package
cmp -s "$OUT_LIBRARY/include/sqlite3.h" "$OUT_HARD/include/sqlite3.h" &&
    cmp -s "$OUT_LIBRARY/include/sqlite3.h" "$OUT_SOFT/include/sqlite3.h" ||
    { echo "package-release: the builds' sqlite3.h differ: they are not one SQLite"; exit 1; }
SQLITE=$(sed -n 's/^#define SQLITE_VERSION  *"\([0-9.]*\)".*/\1/p' "$R/SDK/Include/sqlite3.h")
[ -n "$SQLITE" ] || { echo "package-release: no SQLITE_VERSION in sqlite3.h"; exit 1; }
case "$VERSION" in
    "$SQLITE"-*) ;;
    *) echo "package-release: VERSION $VERSION does not begin with SQLite's $SQLITE-"; exit 1 ;;
esac
# the library says which SQLite it holds
grep -aq "sqlite.library [0-9.]* ([0-9.]*) SQLite $SQLITE" "$R/Libs/sqlite.library" ||
    { echo "package-release: sqlite.library's version text does not name SQLite $SQLITE"; exit 1; }
LIBVER=$(grep -ao 'sqlite.library [0-9][0-9.]* ([0-9.]*)' "$R/Libs/sqlite.library" | head -1)

# SQLite's own notice: the comment at the top of its header, word for word
{
    echo "SQLite is in the public domain. Its header carries this notice (from the top of sqlite3.h, verbatim):"
    echo
    awk '/^\/\*/ { on = 1 } on { print } /\*\// { if (on) exit }' "$R/SDK/Include/sqlite3.h"
} > "$R/Docs/SQLITE-BLESSING.txt"
grep -q "blessing" "$R/Docs/SQLITE-BLESSING.txt" ||
    { echo "package-release: sqlite3.h does not begin with SQLite's notice"; exit 1; }

cat > "$R/ReadMe.txt" <<EOF
GFOpenSQLite $VERSION: SQLite $SQLITE for AmigaOS 3.x, as a shared library
and as link libraries for GCC programs (libnix). AmiCradle Team; SQLite is
public domain.

To install: copy Libs/sqlite.library to LIBS:. That is all a program built
on the library needs. C/SQLiteLibTest checks the install: run it from a
Shell and it ends "SQLITELIBTEST PASS".

Libs/sqlite.library              $LIBVER, 68040, runs with or
                                 without an FPU (uses one when it is there)

For programmers:
SDK/Include/sqlite3.h            SQLite's own header, for either way
SDK/lib/libsqlite3_link.a        on the library: link -lsqlite3_link where
                                 you linked -lsqlite3, no source change
                                 (sqlite3_link.h and Docs/sqlite.library.md)
SDK/lib/libsqlite3.a             SQLite linked in, as before: 68040 with
                                 FPU (-m68040 -m68881), -lsqlite3 -lpthread
SDK/lib/soft-float/              both again with no FPU code, for programs
                                 linked with the soft-float specs and
                                 fpuauto (the --wrap list is fpuauto.wraps)
SDK/sfd/sqlite_lib.sfd           the library's calls, for other compilers

Amiga changes: AmigaDOS paths, an Amiga Lock() as the database lock (the
library and the linked-in SQLite lock each other out of a shared database
correctly), no WAL or mmap, seeks past the end of a file made safe.
Docs/README.md and Docs/sqlite.library.md have the details.

Built from commit $COMMIT.
EOF

mkdir -p "$DEST"
rm -f "$DEST/$NAME.lha" "$DEST/$NAME.zip" "$DEST/SHA256SUMS"
# the same time on every file, so two runs over the same files make the same .zip
find "$R" -exec touch -d "${SOURCE_DATE:-2026-01-01 00:00:00 UTC}" {} +
(cd "$W" && find GFOpenSQLite | LC_ALL=C sort | TZ=UTC zip -q -X -@ "$DEST/$NAME.zip")
if command -v lha >/dev/null 2>&1; then
    (cd "$W" && lha a "$DEST/$NAME.lha" GFOpenSQLite >/dev/null)
    [ -s "$DEST/$NAME.lha" ] || { echo "package-release: lha made no archive"; exit 1; }
else
    echo "package-release: no lha on this computer: $NAME.lha not made"
    [ "${NEED_LHA:-1}" = 0 ] || exit 1
fi
(cd "$DEST" && sha256sum "$NAME".* > SHA256SUMS)
echo "package-release: $NAME from $COMMIT, SQLite $SQLITE, $LIBVER"
(cd "$R" && find . -type f | LC_ALL=C sort | while read -r f; do printf '  %8d %s\n' "$(wc -c < "$f")" "${f#./}"; done)
(cd "$DEST" && ls -l "$NAME".* | awk '{print "  " $5, $9}' && cat SHA256SUMS)
