#!/bin/sh
# gfopensqlite: sqlite.library, SQLite as an AmigaOS shared library, with
# the os32-gcc16 compiler; libsqlite3_link.a, the C API through it; and the
# tests. docs/sqlite.library.md says what is in it and why.
#
#   OS32_GCC16   compiler root holding prefix/ (default
#                ~/AmiCradle/stoves/os32-gcc16), as build.sh
#   OUT          where the results go (default ./out-library)
#   TARBALLS     folder holding the upstream tarball listed in SOURCES
#                (default ./tarballs); its SHA-256 is checked
#   HOST_TESTS=0 skip tests/host-libtest.sh (the link library and the
#                library's side on this computer) and
#                tests/host-amigatest.sh (the library's files, locks, memory
#                and mutexes on this computer)
#
# Results, under $OUT:
#   libs/sqlite.library        copy to LIBS:. One library for every program:
#                              68040, soft float with fpuauto (the FPU when
#                              there is one), safe for any number of tasks.
#   lib/libsqlite3_link.a      for hard-float programs (-m68040 -m68881: the
#                              browser), in place of out/lib/libsqlite3.a
#   lib-soft/libsqlite3_link.a for soft-float programs (-m68040 with
#                              specs/m68k-soft-m68040.specs: GFOpenBase), in
#                              place of out-soft*/lib/libsqlite3.a
#   include/                   sqlite3.h as before, sqlite3_link.h, and
#                              proto/, inline/, libraries/ for a program
#                              that calls the library itself
#   sfd/sqlite_lib.sfd         the calls
#   tests/SQLiteLibTest        tests/libtest.c, hard float; SQLiteLibTestSoft
#                              the same, soft float; SQLiteSmoke is
#                              tests/sqlitetest.c, the static library's smoke
#                              test, unchanged, linked with the link library
#
# The build fails if the table and the generated files disagree, if the
# library needs anything from outside itself or takes C library code in, if
# the compiler makes FPU code for it, or if SQLite calls a floating point
# routine fpuauto does not wrap.
# MIT, Copyright (c) 2026 AmiCradle Team. SQLite keeps its own licence.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
S=${OS32_GCC16:-"$HOME/AmiCradle/stoves/os32-gcc16"}
P=$S/prefix
CC="$P/bin/m68k-amigaos-gcc"
AR="$P/bin/m68k-amigaos-ar"
NM="$P/bin/m68k-amigaos-nm"
OUT=${OUT:-"$HERE/out-library"}
TARBALLS=${TARBALLS:-"$HERE/tarballs"}
SPECS="$HERE/specs/m68k-soft-m68040.specs"
ZIP=sqlite-amalgamation-3530400.zip
SHA=1e71ddf93849c6a6ecf58b827c0692073d2dd7ee40196158068f7b29f422e87d
O="$OUT/obj"

python3 "$HERE/tools/gen-library.py" --check

mkdir -p "$OUT/libs" "$OUT/lib" "$OUT/lib-soft" "$OUT/tests" "$OUT/sfd" "$O" \
    "$OUT/include/libraries" "$OUT/include/inline" "$OUT/include/proto"
[ -f "$TARBALLS/$ZIP" ] || { echo "missing $TARBALLS/$ZIP (see SOURCES)"; exit 2; }
echo "$SHA  $TARBALLS/$ZIP" | sha256sum -c - >/dev/null || { echo "SHA-256 mismatch: $TARBALLS/$ZIP"; exit 2; }
if [ ! -f "$O/sqlite/sqlite-amalgamation-3530400/sqlite3.c" ]; then
    rm -rf "$O/sqlite"; mkdir -p "$O/sqlite"
    (cd "$O/sqlite" && unzip -q "$TARBALLS/$ZIP")
fi
SQ="$O/sqlite/sqlite-amalgamation-3530400"

if [ "${HOST_TESTS:-1}" != 0 ] && command -v "${HOST_CC:-cc}" >/dev/null 2>&1; then
    SQLITE_SRC="$SQ" CC="${HOST_CC:-cc}" sh "$HERE/tests/host-libtest.sh"
    SQLITE_SRC="$SQ" CC="${HOST_CC:-cc}" sh "$HERE/tests/host-amigatest.sh"
fi

cp "$SQ/sqlite3.h" "$HERE/include/sqlite3_link.h" "$OUT/include/"
cp "$HERE/include/libraries/sqlite.h" "$OUT/include/libraries/"
cp "$HERE/include/inline/sqlite.h" "$OUT/include/inline/"
cp "$HERE/include/proto/sqlite.h" "$OUT/include/proto/"
cp "$HERE/library/sqlite_lib.sfd" "$OUT/sfd/"

# ---- the library ----------------------------------------------------------
# Soft float (the specs file links libnix's soft maths), no loops turned into
# memset/memcpy calls, address 0 is memory on an Amiga.
CPU="-m68040 -specs=$SPECS -mcrt=nix20 -fno-tree-loop-distribute-patterns -fno-delete-null-pointer-checks -fomit-frame-pointer"
if echo | "$CC" $CPU -dM -E - | grep -q __HAVE_68881__; then
    echo "sqlite.library: $CC $CPU makes FPU code"; exit 1
fi
CFLAGS="$CPU -O2 -Wall -Wextra -Werror -I$HERE/include -I$HERE/library -I$SQ"
# SQLite as build.sh builds it (no WAL, no mmap, no extension loading), with
# our own files, memory and mutexes (SQLITE_OS_OTHER, SQLITE_ZERO_MALLOC,
# SQLITE_MUTEX_NOOP: library/sqlite_amiga.c gives the real ones at start) and
# no shared cache (one cache per connection, as every program here uses it).
SQFLAGS="$CPU -O2 -I$SQ -DSQLITE_OS_OTHER=1 -DSQLITE_THREADSAFE=1 -DSQLITE_MUTEX_NOOP -DSQLITE_ZERO_MALLOC \
  -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_OMIT_WAL -DSQLITE_MAX_MMAP_SIZE=0 -DSQLITE_DEFAULT_MEMSTATUS=0 \
  -DSQLITE_OMIT_DEPRECATED -DSQLITE_DQS=0 -DSQLITE_OMIT_SHARED_CACHE -DSQLITE_EXTRA_AUTOEXT=sql_amiga_autoext"
if [ ! -f "$O/sqlite3.o" ] || [ "$SQ/sqlite3.c" -nt "$O/sqlite3.o" ] || [ "$0" -nt "$O/sqlite3.o" ]; then
    echo "compiling SQLite ($(sed -n 's/^#define SQLITE_VERSION  *"\(.*\)"/\1/p' "$SQ/sqlite3.h"))"
    # shellcheck disable=SC2086
    "$CC" $SQFLAGS -c "$SQ/sqlite3.c" -o "$O/sqlite3.o"
fi
# shellcheck disable=SC2086
"$CC" $CFLAGS -fno-builtin -c "$HERE/library/sqlite_amiga.c" -o "$O/sqlite_amiga.o"
# shellcheck disable=SC2086
"$CC" $CFLAGS -c "$HERE/library/sqlite_own.c" -o "$O/sqlite_own.o"
# shellcheck disable=SC2086
"$CC" $CFLAGS -fno-toplevel-reorder -c "$HERE/library/sqlite_lib.c" -o "$O/sqlite_lib.o"
"$CC" -m68040 -m68881 -c "$HERE/library/sqlite_stack.S" -o "$O/sqlite_stack.o"

# fpuauto: one member per routine, as amicradle-guest builds libfpuauto.a
rm -f "$O"/fa_*.o "$O/libfpuauto.a"
"$CC" -m68040 -DFPUAUTO_COMMON -c "$HERE/fpuauto/fpuauto.S" -o "$O/fa_common.o"
for n in $(sed 's/^--wrap=//' "$HERE/fpuauto/fpuauto.wraps"); do
    "$CC" -m68040 -DR_$n -c "$HERE/fpuauto/fpuauto.S" -o "$O/fa_$n.o"
done
"$AR" rcs "$O/libfpuauto.a" "$O"/fa_*.o
WRAPS=$(sed 's/^/-Wl,/' "$HERE/fpuauto/fpuauto.wraps" | tr '\n' ' ')

# the FPU check, before linking: every floating point routine our objects
# call is one fpuauto wraps, apart from libgcc's 64-bit integer conversions,
# which fpuauto leaves to libgcc (they call the wrapped routines)
FLOATRE='___(add|sub|mul|div|neg|cmp|eq|ne|lt|le|gt|ge|unord)(s|d|x)f[23]$|___(fix|fixuns)(s|d|x)fsi$|___float(un)?si(s|d|x)f$|___extend|___trunc|^_(sqrt|exp|log|pow|floor|ceil|fmod)$'
UNWRAPPED=
for f in sqlite3 sqlite_amiga sqlite_own sqlite_lib; do
    for s in $("$NM" -u "$O/$f.o" | awk '{print $2}' | grep -E "$FLOATRE" | sort -u || true); do
        r=$(echo "$s" | sed 's/^_//')
        grep -qx -- "--wrap=$r" "$HERE/fpuauto/fpuauto.wraps" || UNWRAPPED="$UNWRAPPED $r"
    done
done
[ -z "$UNWRAPPED" ] || { echo "FPU check: not wrapped by fpuauto:$UNWRAPPED"; exit 1; }

LIB="$OUT/libs/sqlite.library"
# shellcheck disable=SC2086
"$CC" $CPU -nostartfiles -nostdlib -o "$LIB" \
  "$O/sqlite_lib.o" "$O/sqlite_own.o" "$O/sqlite_amiga.o" "$O/sqlite_stack.o" "$O/sqlite3.o" \
  $WRAPS -L"$O" -lfpuauto -lm -lgcc -Wl,-Map,"$O/sqlite.library.map"
echo "$LIB ($(wc -c < "$LIB") bytes)"
if "$NM" -u "$LIB" | grep . ; then
    echo "sqlite.library has undefined symbols"; exit 1
fi
# libnix's maths only: nothing else of the C library may come in
if grep -E '^ *\.text +0x[0-9a-f]+ +0x[0-9a-f]+ .*libnix\.a' "$O/sqlite.library.map"; then
    echo "sqlite.library links C library code"; exit 1
fi
n=$("$NM" "$LIB" | grep -c ' ___wrap_' || true)
"$NM" "$LIB" | grep -q ' ___fpuauto_check' || { echo "FPU check: fpuauto missing"; exit 1; }
echo "FPU check: soft float, $n routines through fpuauto (the FPU when present)"

# ---- the link library, hard float and soft float --------------------------
link_library() {    # directory, CPU flags
    d=$1; shift
    for f in sqlite3_link sqlite3_link_more sqlite3_link_open sqlite3_link_auto; do
        # shellcheck disable=SC2068
        "$CC" $@ -noixemul -O2 -fno-delete-null-pointer-checks -Wall -Wextra -Werror \
            -I"$OUT/include" -I"$HERE/library/link" -c "$HERE/library/link/$f.c" -o "$O/$d-$f.o"
    done
    rm -f "$OUT/$d/libsqlite3_link.a"
    "$AR" rcs "$OUT/$d/libsqlite3_link.a" "$O/$d-sqlite3_link.o" "$O/$d-sqlite3_link_more.o" \
        "$O/$d-sqlite3_link_open.o" "$O/$d-sqlite3_link_auto.o"
    echo "$OUT/$d/libsqlite3_link.a ($(wc -c < "$OUT/$d/libsqlite3_link.a") bytes)"
}
link_library lib -m68040 -m68881
link_library lib-soft -m68040 "-specs=$SPECS"

# ---- the tests -------------------------------------------------------------
T="-noixemul -O2 -fno-delete-null-pointer-checks -Wall -Wextra -Werror -I$OUT/include -I$HERE/library/link"
# shellcheck disable=SC2086
"$CC" -m68040 -m68881 $T -o "$OUT/tests/SQLiteLibTest" "$HERE/tests/libtest.c" -L"$OUT/lib" -lsqlite3_link
# shellcheck disable=SC2086
"$CC" -m68040 "-specs=$SPECS" $T -o "$OUT/tests/SQLiteLibTestSoft" "$HERE/tests/libtest.c" -L"$OUT/lib-soft" -lsqlite3_link
# the static library's own smoke test, not a line changed
# shellcheck disable=SC2086
"$CC" -m68040 -m68881 -noixemul -O2 -I"$OUT/include" -o "$OUT/tests/SQLiteSmoke" "$HERE/tests/sqlitetest.c" -L"$OUT/lib" -lsqlite3_link
for f in SQLiteLibTest SQLiteLibTestSoft SQLiteSmoke; do
    echo "$OUT/tests/$f ($(wc -c < "$OUT/tests/$f") bytes)"
    # a program linked with the link library has no SQLite of its own
    if "$NM" "$OUT/tests/$f" | grep -q ' _sqlite3VdbeExec'; then
        echo "$f has SQLite linked in"; exit 1
    fi
done
(cd "$OUT" && sha256sum libs/sqlite.library lib/libsqlite3_link.a lib-soft/libsqlite3_link.a \
    tests/SQLiteLibTest tests/SQLiteLibTestSoft tests/SQLiteSmoke > SHA256SUMS)
cat "$OUT/SHA256SUMS"
