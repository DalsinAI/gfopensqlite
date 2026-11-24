#!/bin/sh
# gfopensqlite: the release package in one kitchen job (stove_build, target ., script build-release.sh).
# The three builds tools/package-release.sh takes (build.sh, FLOAT=soft build.sh, build-library.sh, which
# runs the host tests), what built them in OUT/BUILD-INFO.txt, then the package itself in OUT:
# GFOpenSQLite-<VERSION>.lha and .zip with SHA256SUMS. Publishing is not done here: a release is Dale's word.
#
#   VERSION    the release's name (default: the first line of ./VERSION, as 3.53.4-2)
#   OUT        where the package goes (default ./release; the kitchen gives its job's out/)
#   STOVE      the compiler root holding prefix/ (default ~/AmiCradle/stoves/os32-gcc16); build.sh and
#              build-library.sh take it as OS32_GCC16
#   TARBALLS   folder for the upstream file SOURCES names (default ./tarballs); fetched from SOURCES' address
#              when it isn't there, and build.sh checks its SHA-256 either way
#
# usage: ./build-release.sh
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
OUT=${OUT:-$HERE/release}
STOVE=${STOVE:-$HOME/AmiCradle/stoves/os32-gcc16}
TARBALLS=${TARBALLS:-$HERE/tarballs}
if [ -z "${VERSION:-}" ]; then
    [ -f "$HERE/VERSION" ] || { echo "build-release: no VERSION file and no VERSION set"; exit 1; }
    VERSION=$(head -n 1 "$HERE/VERSION" | tr -d '[:space:]')
fi
[ -n "$VERSION" ] || { echo "build-release: VERSION is empty"; exit 1; }
[ -x "$STOVE/prefix/bin/m68k-amigaos-gcc" ] || { echo "build-release: no compiler in $STOVE/prefix/bin"; exit 1; }
# the commit is taken before anything is built, while the tracked files are still HEAD's
COMMIT=${COMMIT:-$(git -C "$HERE" rev-parse --short=7 HEAD)}
[ -z "$(git -C "$HERE" status --porcelain --untracked-files=no)" ] ||
    { echo "build-release: files differ from HEAD $COMMIT: build a commit, not a changed tree"; exit 1; }

# the upstream file, once: SOURCES' file and from lines
FILE=$(sed -n 's/^ *file: *//p' "$HERE/SOURCES" | head -n 1)
FROM=$(sed -n 's/^ *from: *//p' "$HERE/SOURCES" | head -n 1)
mkdir -p "$TARBALLS"
if [ ! -f "$TARBALLS/$FILE" ]; then
    curl -sSfL -o "$TARBALLS/$FILE.part" "$FROM" || { rm -f "$TARBALLS/$FILE.part"; echo "build-release: couldn't fetch $FROM"; exit 1; }
    mv "$TARBALLS/$FILE.part" "$TARBALLS/$FILE"
fi

export OS32_GCC16="$STOVE" TARBALLS
sh "$HERE/build.sh"
FLOAT=soft sh "$HERE/build.sh"
# build-library.sh takes OUT as its own folder: the kitchen's OUT is for the package, so it gets its default
OUT="$HERE/out-library" sh "$HERE/build-library.sh"

mkdir -p "$OUT"
VERSION=$VERSION COMMIT=$COMMIT OUT_LIBRARY="$HERE/out-library" DEST=$OUT sh "$HERE/tools/package-release.sh"
{
    echo "GFOpenSQLite $VERSION"
    echo "Source: $(git -C "$HERE" rev-parse HEAD)"
    echo "Stove: $(basename "$STOVE")"
    echo "Compiler: $("$STOVE/prefix/bin/m68k-amigaos-gcc" --version | head -n 1)"
} > "$OUT/BUILD-INFO.txt"
cat "$OUT/BUILD-INFO.txt"
