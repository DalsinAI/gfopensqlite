#!/bin/sh
# gfopensqlite: SQLite, built for AmigaOS 3.x (68040 + FPU) with the
# os32-gcc16 compiler (bebbo's amiga-gcc, GCC 16.2, libnix, libpthread).
# MIT, Copyright (c) 2026 AmiCradle Team. The library keeps its own licence.
#
#   OS32_GCC16   compiler root holding prefix/ and compat/
#                (default ~/AmiCradle/stoves/os32-gcc16)
#   FLOAT        hard (default: -m68040 -m68881, the browser stack) or soft
#                (-m68040 alone, no FPU code: for programs linked soft float
#                with fpuauto, as FileZilla is; the build fails if SQLite
#                calls a floating point routine fpuauto/fpuauto.wraps lacks)
#   PREFIX       where include/ and lib/ go (default ./out, ./out-soft for
#                FLOAT=soft)
#   TARBALLS     folder holding the upstream tarballs listed in SOURCES
#                (default ./tarballs); the script checks their SHA-256
#   JOBS         parallel jobs for CMake/make builds (default 2)
#
# usage: ./build.sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
S=${OS32_GCC16:-"$HOME/AmiCradle/stoves/os32-gcc16"}
P=$S/prefix
FLOAT=${FLOAT:-hard}
case "$FLOAT" in
    hard) OUT=${PREFIX:-"$HERE/out"}; WORK="$HERE/work"
          CPU=${OS32_CPU_FLAGS:-"-m68040 -m68881 -mcrt=nix20"} ;;
    soft) OUT=${PREFIX:-"$HERE/out-soft"}; WORK="$HERE/work-soft"
          CPU="-m68040 -mcrt=nix20" ;;
    *) echo "FLOAT is hard or soft"; exit 2 ;;
esac
TARBALLS=${TARBALLS:-"$HERE/tarballs"}
JOBS=${JOBS:-2}
CC="$P/bin/m68k-amigaos-gcc"
CXX="$P/bin/m68k-amigaos-g++"
AR="$P/bin/m68k-amigaos-ar"
NM="$P/bin/m68k-amigaos-nm"
if [ "$FLOAT" = soft ] && echo | "$CC" $CPU -dM -E - | grep -q __HAVE_68881__; then
    echo "FLOAT=soft: $CC $CPU makes FPU code"; exit 1
fi
CFLAGS="-O2 $CPU -fno-delete-null-pointer-checks -D_DEFAULT_SOURCE=1 -D_POSIX_TIMERS=1 -D_POSIX_REALTIME_SIGNALS=1 -fno-common"
mkdir -p "$OUT/include" "$OUT/lib" "$WORK"

# unpack NAME TARBALL SHA256: check the tarball and unpack it into $WORK
unpack() {
    t="$TARBALLS/$2"
    [ -f "$t" ] || { echo "missing $t (see SOURCES)"; exit 2; }
    echo "$3  $t" | sha256sum -c - >/dev/null || { echo "SHA-256 mismatch: $t"; exit 2; }
    rm -rf "$WORK/$1"; mkdir -p "$WORK/$1"
    case "$2" in
        *.zip) (cd "$WORK/$1" && unzip -q "$t") ;;
        *) tar xf "$t" -C "$WORK/$1" ;;
    esac
}

# archive NAME FILE...: compile into $OUT/lib/libNAME.a ($XFLAGS added)
archive() {
    name=$1; shift
    obj="$WORK/obj-$name"
    rm -rf "$obj"; mkdir -p "$obj"
    for f in "$@"; do
        o="$obj/$(echo "$f" | tr '/' '_' | sed 's/\.[a-z]*$//').o"
        case "$f" in
            *.cc|*.cpp) $CXX $CFLAGS ${XFLAGS:-} -c "$f" -o "$o" ;;
            *) $CC $CFLAGS ${XFLAGS:-} -c "$f" -o "$o" ;;
        esac
    done
    rm -f "$OUT/lib/lib$name.a"
    $AR rcs "$OUT/lib/lib$name.a" "$obj"/*.o
    echo "lib$name.a: $(wc -c < "$OUT/lib/lib$name.a") bytes"
}

unpack sqlite sqlite-amalgamation-3530400.zip 1e71ddf93849c6a6ecf58b827c0692073d2dd7ee40196158068f7b29f422e87d
cd "$WORK/sqlite/sqlite-amalgamation-3530400"
patch -p1 < "$HERE/patches/sqlite-3.53.4-amiga-paths.patch"
# AmigaDOS refuses a seek past the end of a file on a real disk: a read
# there reads nothing, a write there grows the file with zeros first
patch -p1 < "$HERE/patches/sqlite-3.53.4-amiga-seek.patch"
# One process, no shared memory: no WAL and no mmap.
# Locking: libnix has no POSIX file locks (its fcntl() always fails), so the
# default VFS is "unix-dotfile", whose lock the patch makes an exclusive Lock()
# on "<database>.lck" (compat/sqlite_amiga.c; gone after a reset, so none is
# left behind), and every connection waits up to 2 s for it
# (SQLITE_EXTRA_AUTOEXT, compat/sqlite_amiga.c). A program that keeps its own
# lock can still pass "unix-none" to sqlite3_open_v2(), as GFOpenBase does.
# compat/sqlite_amiga_pre.h keeps exec/types.h's GLOBAL out of SQLite's way.
XFLAGS="-DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_OMIT_WAL -DSQLITE_MAX_MMAP_SIZE=0 \
    -DSQLITE_DEFAULT_MEMSTATUS=0 -DSQLITE_OMIT_DEPRECATED -DSQLITE_DQS=0 -DHAVE_USLEEP=1 -DSQLITE_OS_UNIX=1 \
    -DSQLITE_DEFAULT_UNIX_VFS=\"unix-dotfile\" -DSQLITE_EXTRA_AUTOEXT=sqlite_amiga_autoext -I. \
    -include $HERE/compat/sqlite_amiga_pre.h" \
    archive sqlite3 sqlite3.c "$HERE/compat/sqlite_amiga.c"
cp sqlite3.h sqlite3ext.h "$OUT/include/"

# FLOAT=soft, the FPU check: every floating point routine SQLite calls is one
# fpuauto wraps (so a program linked with fpuauto takes the FPU when there is
# one), apart from libgcc's 64-bit integer conversions, which fpuauto leaves
# to libgcc (they call the wrapped routines)
if [ "$FLOAT" = soft ]; then
    FLOATRE='___(add|sub|mul|div|neg|cmp|eq|ne|lt|le|gt|ge|unord)(s|d|x)f[23]$|___(fix|fixuns|float|floatun)(s|d|x)(i|d)f$|___(fix|fixuns)(s|d|x)fsi$|___float(un)?si(s|d|x)f$|___extend|___trunc|^_(sqrt|exp|log|pow|floor|ceil|fmod)$'
    UNWRAPPED=
    for s in $("$NM" -u "$OUT/lib/libsqlite3.a" | awk '{print $2}' | grep -E "$FLOATRE" | sort -u || true); do
        r=$(echo "$s" | sed 's/^_//')
        grep -qx -- "--wrap=$r" "$HERE/fpuauto/fpuauto.wraps" || UNWRAPPED="$UNWRAPPED $r"
    done
    [ -z "$UNWRAPPED" ] || { echo "FPU check: not wrapped by fpuauto:$UNWRAPPED"; exit 1; }
    n=$("$NM" -u "$OUT/lib/libsqlite3.a" | awk '{print $2}' | sed 's/^_//' | sort -u | while read -r r; do
        grep -qx -- "--wrap=$r" "$HERE/fpuauto/fpuauto.wraps" && echo "$r"; done | wc -l)
    echo "FPU check: soft float, no FPU code, $n routines for fpuauto to wrap"
fi

