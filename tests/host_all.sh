#!/bin/sh
# Every host test of sqlite.library, as pull request CI runs them (tests/ci/extra.txt):
# fetches the SQLite amalgamation named in build-library.sh (SHA-256 checked) unless
# SQLITE_SRC already points at it, then runs each tests/host-*.sh.
#   tests/host_all.sh       exit 77 when the amalgamation can't be fetched
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")/.." && pwd)
ZIP=$(sed -n 's/^ZIP=//p' "$HERE/build-library.sh")
SHA=$(sed -n 's/^SHA=//p' "$HERE/build-library.sh")
W=$(mktemp -d); trap 'rm -rf "$W"' EXIT
if [ -z "${SQLITE_SRC:-}" ]; then
    Z=${TARBALLS:-$HERE/tarballs}/$ZIP
    if [ ! -f "$Z" ]; then
        Z=$W/$ZIP
        curl -fsSL -o "$Z" "https://www.sqlite.org/2026/$ZIP" || { echo "SKIP: can't fetch $ZIP"; exit 77; }
    fi
    echo "$SHA  $Z" | sha256sum -c - >/dev/null || { echo "SHA-256 mismatch: $Z"; exit 1; }
    (cd "$W" && unzip -q "$Z")
    SQLITE_SRC=$W/${ZIP%.zip}
fi
export SQLITE_SRC
fails=0
for t in libtest amigatest locktest seektest baseprobe packagetest releasetest; do
    if sh "$HERE/tests/host-$t.sh"; then echo "PASS host-$t"; else echo "FAIL host-$t"; fails=$((fails + 1)); fi
done
[ "$fails" = 0 ]
