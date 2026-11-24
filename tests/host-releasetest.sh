#!/bin/sh
# gfopensqlite: build-release.sh, the kitchen's one job for a release package, over stand-in builds.
#   the three builds run, each with the stove as OS32_GCC16 (build.sh twice: hard, then FLOAT=soft);
#   the package lands in OUT under VERSION's name, with SHA256SUMS, and BUILD-INFO.txt names the version,
#   the commit, the stove and its compiler; VERSION set in the environment wins over the file;
#   the upstream file is fetched from SOURCES' address only when TARBALLS hasn't got it;
#   no VERSION, no compiler, and a tracked file that differs from HEAD each stop it before a build.
# usage: tests/host-releasetest.sh
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
W=$(mktemp -d)
trap 'rm -rf "$W"' EXIT
checks=0; failed=0
check() { checks=$((checks + 1)); if ! eval "$2"; then failed=$((failed + 1)); echo "FAIL: $1"; fi; }

# a stove: prefix/bin/m68k-amigaos-gcc that only says what it is
mkdir -p "$W/stove-x/prefix/bin"
printf '#!/bin/sh\necho "m68k-amigaos-gcc (Test 16.2.0-open9) 16.2.0b"\n' > "$W/stove-x/prefix/bin/m68k-amigaos-gcc"
chmod +x "$W/stove-x/prefix/bin/m68k-amigaos-gcc"
# the upstream file, for a fetch by file: address
echo upstream > "$W/upstream.zip"

# tree: the repository's own release files, the builds replaced by stand-ins that record how they were run
tree() {
    T=$W/tree
    rm -rf "$T" "$W/out" "$W/runs"
    mkdir -p "$T/tools" "$T/library" "$T/fpuauto" "$T/docs" "$T/tarballs"
    cp "$HERE/build-release.sh" "$HERE/VERSION" "$HERE/README.md" "$HERE/LICENSE" "$HERE/CONTRIBUTORS.md" "$T/"
    cp "$HERE/tools/package-release.sh" "$T/tools/"
    cp "$HERE/library/sqlite_lib.sfd" "$T/library/"; cp "$HERE/fpuauto/fpuauto.wraps" "$T/fpuauto/"
    cp "$HERE/docs/sqlite.library.md" "$T/docs/"
    printf 'SQLite 3.53.4\n  file:   upstream.zip\n  from:   file://%s/upstream.zip\n' "$W" > "$T/SOURCES"
    cp "$W/upstream.zip" "$T/tarballs/"
    printf '/out/\n/out-soft/\n/out-library/\n/tarballs/\n/release/\n' > "$T/.gitignore"
    cat > "$T/build.sh" <<'STUB'
#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
echo "build.sh FLOAT=${FLOAT:-hard} OS32_GCC16=$OS32_GCC16 TARBALLS=$TARBALLS" >> "$HERE/../runs"
case "${FLOAT:-hard}" in hard) O=$HERE/out ;; soft) O=$HERE/out-soft ;; esac
mkdir -p "$O/include" "$O/lib"
printf '/*\n** The author disclaims copyright to this source code.  In place of\n** a legal notice, here is a blessing:\n**\n**    May you do good and not evil.\n*/\n#define SQLITE_VERSION        "3.53.4"\n' > "$O/include/sqlite3.h"
echo ext > "$O/include/sqlite3ext.h"; echo "${FLOAT:-hard}" > "$O/lib/libsqlite3.a"
STUB
    cat > "$T/build-library.sh" <<'STUB'
#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
echo "build-library.sh OS32_GCC16=$OS32_GCC16" >> "$HERE/../runs"
O=$HERE/out-library
mkdir -p "$O/libs" "$O/tests" "$O/lib" "$O/lib-soft" "$O/include/libraries" "$O/include/inline" "$O/include/proto"
cp "$HERE/out/include/sqlite3.h" "$O/include/"
printf 'xx\0$VER: sqlite.library 1.0 (24.11.2026) SQLite 3.53.4, GFOpenSQLite\0yy' > "$O/libs/sqlite.library"
echo test > "$O/tests/SQLiteLibTest"; echo link > "$O/include/sqlite3_link.h"
for d in libraries inline proto; do echo "$d" > "$O/include/$d/sqlite.h"; done
echo lh > "$O/lib/libsqlite3_link.a"; echo ls > "$O/lib-soft/libsqlite3_link.a"
STUB
    (cd "$T" && git init -q && git add -A && git -c user.name=test -c user.email=test@example.invalid commit -q -m tree)
}
run() {     # extra environment as NAME=value words, then the script's log in $W/log
    rc=0
    env OUT="$W/out" STOVE="$W/stove-x" NEED_LHA=0 "$@" sh "$W/tree/build-release.sh" > "$W/log" 2>&1 || rc=$?
}

tree
V=$(head -n 1 "$HERE/VERSION")
run
check "it builds" '[ $rc = 0 ]'
check "the package in OUT, named by VERSION, with its sums" '[ -s "$W/out/GFOpenSQLite-$V.zip" ] && (cd "$W/out" && sha256sum -c SHA256SUMS >/dev/null 2>&1)'
check "build.sh hard, build.sh soft, build-library.sh, in that order" '[ "$(cut -d" " -f1,2 "$W/runs" | tr "\n" ";")" = "build.sh FLOAT=hard;build.sh FLOAT=soft;build-library.sh OS32_GCC16=$W/stove-x;" ]'
check "every build is given the stove and the tarballs" '[ "$(grep -c "OS32_GCC16=$W/stove-x" "$W/runs")" = 3 ] && grep -q "TARBALLS=$W/tree/tarballs" "$W/runs"'
H=$(git -C "$W/tree" rev-parse HEAD)
check "BUILD-INFO.txt: version, commit, stove, compiler" 'grep -qx "GFOpenSQLite $V" "$W/out/BUILD-INFO.txt" && grep -qx "Source: $H" "$W/out/BUILD-INFO.txt" && grep -qx "Stove: stove-x" "$W/out/BUILD-INFO.txt" && grep -q "^Compiler: m68k-amigaos-gcc (Test 16.2.0-open9)" "$W/out/BUILD-INFO.txt"'
check "the ReadMe names the commit" 'unzip -p "$W/out/GFOpenSQLite-$V.zip" GFOpenSQLite/ReadMe.txt | grep -q "Built from commit $(git -C "$W/tree" rev-parse --short=7 HEAD)"'
check "the upstream file was there, so nothing was fetched" '! grep -q fetch "$W/log"'

tree
run VERSION=3.53.4-2-rc9
check "VERSION in the environment wins" '[ $rc = 0 ] && [ -s "$W/out/GFOpenSQLite-3.53.4-2-rc9.zip" ] && grep -qx "GFOpenSQLite 3.53.4-2-rc9" "$W/out/BUILD-INFO.txt"'

tree; rm "$W/tree/tarballs/upstream.zip"
run
check "no upstream file: fetched from SOURCES' address" '[ $rc = 0 ] && cmp -s "$W/tree/tarballs/upstream.zip" "$W/upstream.zip"'
tree; rm "$W/tree/tarballs/upstream.zip"; sed -i "s#file://$W/upstream.zip#file://$W/no-such.zip#" "$W/tree/SOURCES"
(cd "$W/tree" && git -c user.name=test -c user.email=test@example.invalid commit -qam elsewhere)
run
check "a fetch that fails stops it before a build" '[ $rc != 0 ] && grep -q "couldn.t fetch" "$W/log" && [ ! -e "$W/runs" ] && [ ! -e "$W/tree/tarballs/upstream.zip.part" ]'

tree; (cd "$W/tree" && git rm -q VERSION && git -c user.name=test -c user.email=test@example.invalid commit -qm noversion)
run
check "no VERSION stops it before a build" '[ $rc != 0 ] && grep -q "no VERSION" "$W/log" && [ ! -e "$W/runs" ]'
tree
run STOVE="$W/no-stove"
check "no compiler stops it before a build" '[ $rc != 0 ] && grep -q "no compiler" "$W/log" && [ ! -e "$W/runs" ]'
tree; echo changed >> "$W/tree/README.md"
run
check "a tracked file that differs from HEAD stops it before a build" '[ $rc != 0 ] && grep -q "differ from HEAD" "$W/log" && [ ! -e "$W/runs" ] && [ ! -e "$W/out/GFOpenSQLite-$V.zip" ]'

if [ $failed = 0 ]; then echo "RELEASETEST PASS: $checks checks"; else echo "RELEASETEST FAIL: $failed of $checks"; exit 1; fi
