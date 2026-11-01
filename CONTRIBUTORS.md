# Contributors

## Creator and maintainer

- **SacredTrees** ([@SacredTrees](https://github.com/SacredTrees)): created and maintains this AmigaOS port of SQLite (openamigasqlite).

## The AmiCradle team

We are The AmiCradle Team, who build AmiCradle alongside SacredTrees:

- **Agnus**, our coordinator, who keeps every thread moving.
- **Thufir**, **Kynes** and **Galen**, who started the work on SacredTrees's x86 cores.
- **The threads**, each one taking a piece of the work from design to release.

## Copyright holder

Our Amiga work here (the build script, `compat/sqlite_amiga.c` and the
tests) is Copyright (c) 2026 AmiCradle Team, released under the MIT licence
(`LICENSE`). SQLite itself is not ours: its authors dedicated it to the
public domain, and the change our patch makes to `sqlite3.c`
(`patches/sqlite-3.53.4-amiga-paths.patch`) stays under SQLite's terms.

## Third-party work in this repository

Only SQLite's notice is committed here; its source is not.

| Component | Where | Authors | Licence |
| --- | --- | --- | --- |
| SQLite blessing and amalgamation notice | `upstream/SQLITE-BLESSING.txt` | D. Richard Hipp and the SQLite developers | Public domain |

## Fetched at build time, not committed

`build.sh` unpacks this archive, listed with its SHA-256 sum in `SOURCES`:

- **SQLite 3.53.4** amalgamation (`sqlite-amalgamation-3530400.zip`): D. Richard Hipp and the SQLite developers, public domain.

## Used at build time, not included

- **bebbo's amiga-gcc** (GCC 16.2 with libnix and libpthread), the os32-gcc16 compiler, under its own licences.

Amiga, AmigaOS and other product names are trademarks of their respective
owners.
