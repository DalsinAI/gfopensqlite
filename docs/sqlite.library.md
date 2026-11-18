# sqlite.library

SQLite 3.53.4 as one AmigaOS shared library, in `LIBS:`, for every program
that today links its own copy of `libsqlite3.a` (about 900 KB each in the
browser, GFOpenBase, GFOffice and GFOpenTransMem).

A program changes one thing: it links `-lsqlite3_link` where it linked
`-lsqlite3`. It still includes `<sqlite3.h>` and calls `sqlite3_open()` and
the rest. `build-library.sh` builds the library, the link library and the
tests; the static libraries (`build.sh`) are unchanged and still built.

| File | What it is |
| --- | --- |
| `libs/sqlite.library` | The library. 68040, soft float with fpuauto (the FPU when there is one), no C library inside. |
| `lib/libsqlite3_link.a` | The C API through the library, for hard-float programs (`-m68040 -m68881`: the browser). |
| `lib-soft/libsqlite3_link.a` | The same for soft-float programs (`-m68040 -specs=specs/m68k-soft-m68040.specs`: GFOpenBase). |
| `include/sqlite3.h` | Upstream's header, as before. |
| `include/sqlite3_link.h` | For a program that opens the library with its own requester (below). |
| `include/proto/sqlite.h`, `inline/`, `libraries/`, `sfd/sqlite_lib.sfd` | For a program that calls the library itself. |

There is one library and one pair of link libraries, where the static build
has four (`out`, `out-soft`, `out-st`, `out-soft-st`). Hard or soft float is
now only a matter of how the link library's own few lines are compiled, and
the single-task builds have no successor: the library is always safe for
many tasks, and a program no longer links libpthread for SQLite's sake.

## Which calls

The table in `tools/gen-library.py` has 141 calls. It was made from what the
programs call, counted in their source on 18 November 2026:

| Program | Where | Calls |
| --- | --- | --- |
| GFOffice Base and Calc external data | gfoffice `src/base`, `src/calc/extdb` | 49 |
| GFOpenBase | gfopenbase `src` | 47 |
| The browser | WebKit `Source/WebCore/platform/sql` | 54 |
| GFOpenTransMem | gfopentransmem `src`, `library`, `command` | 29 |
| ACFind (Emu68 path) | design only: open read-only, prepare, step, columns | |

Every call those use is in the table or in the link library, apart from the
four in "Left out" below. The table also has the calls that go with them
(`sqlite3_bind_text64` beside `sqlite3_bind_text`, the other result and
value calls, the hooks), so the next program does not need a new library
version for an obvious neighbour. A call can be added later, at the end of
the table, with the next version number; nothing is ever reordered or
removed.

### Left out

- `sqlite3_vfs_register`, `sqlite3_os_init`, `sqlite3_os_end` (GFOpenTransMem's
  own file layer): a VFS is a table of the program's functions and structures
  that every other program's SQLite would then share. The library's own files
  do what that layer did (it began as GFOpenTransMem's).
- `sqlite3_load_extension`, WAL, mmap: not in the static build either.
- `sqlite3_temp_directory`: a variable, not a call. The link library has one
  so a program that sets it still builds (GFOpenBase), but the library's
  temporary files go to `T:` whatever it says.
- The UTF-16 calls, virtual tables, incremental blob I/O, the backup calls,
  `sqlite3_get_table`: no program here uses them.

## What a shared library changes, and how each is handled

### Arguments in registers

A library call takes its arguments in registers. `tools/gen-library.py` makes
each call from its C prototype: pointers in a0 to a3, then everything else in
d0 to d7 (a call with more than four pointers puts the rest in data
registers). The library names are `SQL_` and the C name in capitals
(`sqlite3_bind_int64` is `SQL_BindInt64`), so the inline macros never clash
with `sqlite3.h`.

- **64-bit values.** An argument is two registers, high half then low half.
  A result is written through a pointer the program passes as the last
  argument. The link library splits and joins them, so
  `sqlite3_column_int64()` is unchanged for the program.
- **Doubles.** Passed by pointer in both directions. A hard-float program
  expects a double result in fp0 and a soft-float one in d0/d1; the library
  is soft float and could satisfy only one. By pointer there is nothing to
  agree on, which is why one library serves both kinds of program. The link
  library is compiled both ways, so `sqlite3_column_double()` returns the
  double the way its program expects.
- **Function pointers** cross as plain addresses (next section).

### Calls from the library back into the program

SQLite calls the program's functions: `sqlite3_exec`'s row function, SQL
functions and aggregates, collations, destructors (`sqlite3_bind_text`'s
last argument), the busy, progress, authorizer, commit, rollback, update and
trace hooks. The library calls each as an ordinary C function, arguments on
the stack, on the task that made the SQLite call, with that task's stack (or
the bigger one the library gave the call, below). Nothing is wrapped, so
`SQLITE_STATIC` and `SQLITE_TRANSIENT` (0 and -1) pass through as they are,
and a program may pass `sqlite3_free` itself as a destructor: that is the
link library's function, which calls the library.

**A4 and small data.** The library keeps no global register: it is built
large-data, and inside it a4 is an ordinary register that SQLite's code uses
for its own values. A callback is therefore entered with a4 holding whatever
the library last put there, not the program's data pointer. A program built
with small data (`-fbaserel`, `-msmall-data`) must mark every function it
hands to SQLite `__saveds`, which reloads a4 on entry, as it must for hooks
and interrupt code. A program built large-data needs nothing, and that is
GCC's default here and what every program in the survey uses. The link
library itself is built large-data and is not for small-data programs as it
stands (it would need building `-fbaserel` like the program).

A callback's own 64-bit and double arguments (the update hook's rowid) are on
the stack, where hard and soft float agree.

### A variable number of arguments

`sqlite3_mprintf`, `sqlite3_snprintf`, `sqlite3_str_appendf`,
`sqlite3_db_config` and `sqlite3_config` take "...", which registers can't
carry. They are ordinary functions in the link library
(`library/link/sqlite3_link_more.c`):

- The three formatting calls take the list in the program and pass it on as
  a `va_list` to `sqlite3_vmprintf`, `sqlite3_vsnprintf` and
  `sqlite3_str_vappendf`, which are in the table. A `va_list` is a pointer
  into the program's stack, where an int is 4 bytes, a 64-bit integer 8 and a
  double 8 in hard-float and soft-float code alike, so SQLite reads it as it
  always did. `sqlite3_vmprintf` and the others are also there for programs
  that call them directly (GFOffice does).
- `sqlite3_db_config` unpacks its arguments by option (a name; a buffer and
  two numbers; or a number and a pointer for all the rest) and calls
  `SQL_DbConfig` with them in fixed places.
- `sqlite3_config` does not reach the library (next section).

### State that belongs to everyone

One copy of SQLite now serves every program, so what was per-program is
shared:

- **Start and end.** The library starts SQLite when the first program opens
  it and shuts it down when the last one closes it. `sqlite3_initialize()`
  answers `SQLITE_OK` and `sqlite3_shutdown()` answers `SQLITE_OK` and does
  nothing: one program must not shut SQLite down under the others.
- **`sqlite3_config`.** The three threading options answer `SQLITE_OK` (the
  library is serialized, which is safe for whatever was asked). Everything
  else answers `SQLITE_MISUSE`, which is what SQLite itself answers once it
  is running. The browser asks for `SQLITE_CONFIG_LOOKASIDE` and for its own
  allocator (`SQLITE_CONFIG_MALLOC`, fastMalloc): both are now refused, and
  WebKit only logs that (`SQLiteDatabase::useFastMalloc`). The browser's
  SQLite memory then comes from the library's pool, not from fastMalloc.
- **`sqlite3_soft_heap_limit64`** answers 0 and does nothing: there is one
  heap for everyone. It did nothing in the static build either, which is
  built without the memory statistics the limit works from, as this is.
- **Per-connection settings** (`sqlite3_db_config`, `sqlite3_busy_timeout`,
  `sqlite3_limit`, `sqlite3_extended_result_codes`, pragmas, functions,
  collations, hooks) belong to the connection, as always. One program's SQL
  functions are not visible on another program's connections.
- **No shared cache** (`SQLITE_OMIT_SHARED_CACHE`): two programs never share
  pages in memory. They meet only at the file lock, as two programs with
  their own SQLite did.

### Whose connection is whose

AmigaOS does not close a program's files or free its locks when it ends, and
with the SQLite code no longer in the program, its exit code can't either.
So the library keeps track (`library/sqlite_own.c`):

- When the link library opens `sqlite.library` (before `main()`), it asks
  for an **owner token** (`SQL_Attach`). `sqlite3_open` and
  `sqlite3_open_v2` pass the token with the call, and the library puts the
  new connection on that owner's list. `sqlite3_close` takes it off.
- When the program ends (the link library's exit code), `SQL_Detach` closes
  what is still on the list: each connection's statements are finalized, its
  transaction is rolled back, its lock dropped and its files closed.
- A token stands for the **program**, not for a task. Any task of the
  program may use or close a connection that another of its tasks opened;
  the library never asks which task is calling. The browser opens databases
  on worker threads and closes them from others, and SQLite's serialized
  mode makes that safe.
- Memory a program allocated and never freed (`sqlite3_malloc`,
  `sqlite3_mprintf`) stays in the library's pool until the last program
  closes the library, when the whole pool goes.
- A program that crashes never detaches. Its connections stay open and its
  lock stays held until a reset, as with the static library.

### Memory

SQLite's memory is one exec pool (`CreatePool`), guarded by a semaphore,
each block with its size in front. It belongs to the library, not to the
task that asked, so memory allocated on one task can be freed on another and
outlives the task. The pool is deleted when the last program closes the
library.

### Files and locks

SQLite is built with `SQLITE_OS_OTHER` and the library's own two VFSes on
dos.library (`library/sqlite_amiga.c`), instead of the unix VFS over libnix
that the static build patches:

- **`unix-dotfile`**, the default: the lock is an exclusive `Lock()` on
  `<database>.lck`, the same file and the same lock as the static build's, so
  a program using the library and one still linked statically keep each
  other out of the same database correctly. Every connection waits up to
  2 s for it, as before (`sqlite3_busy_timeout` changes that).
- **`unix-none`**: no lock, for a program that keeps its own (GFOpenBase
  passes this name to `sqlite3_open_v2`).

They keep the static build's names so nothing a program passes has to
change. AmigaDOS names are used as given; a relative name is made whole from
its drawer (`NameFromLock`) so the journal stays beside the database. A seek
past the end of a file, which AmigaDOS refuses on a real disk, is handled as
the static build's patch does: a read there reads nothing and a write first
grows the file. Temporary files go to `T:`. Files are limited to 2 GB.

Callers must be **processes**, not bare tasks: the file work runs on the
caller's task and dos.library needs a process.

### Tasks

SQLite runs serialized (`SQLITE_THREADSAFE=1`) with SignalSemaphores for its
mutexes, so any number of programs and tasks may call at once, and two tasks
may use one connection. This holds for every program, including those that
used the single-task static build.

### Stack

SQLite needs more stack than a Shell's 4 KB. A call that can run SQL (open,
close, prepare, step, exec, finalize, reset and a few more: `deep` in the
table) checks the caller's stack and, when under 48 KB is free, runs on a
128 KB stack of its own (`StackSwap`) for that call. Callbacks made during
the call run on that stack. A program with a big enough stack pays one
comparison.

### Floating point

The library is soft float, so it runs on a 68LC040 or 68EC040, and every
floating point routine goes through fpuauto, which uses the FPU when there
is one. The build fails if SQLite calls a routine fpuauto does not wrap or
if the compiler emits FPU code for the library. Opening the maths libraries
changes the opening task's FPU rounding precision; the library puts the
program's setting back, since a hard-float program does its own doubles on
the FPU.

### The clock

`'now'` is UTC: the Amiga's clock moved by the locale's offset from GMT
(locale.library, read when SQLite starts). The `'localtime'` modifier moves
it back. Without locale.library the clock is taken as UTC.

### WAL

The build has no WAL (`SQLITE_OMIT_WAL`), as the static build has none:
AmigaOS has no shared memory to put the index in. What the calls answer:

- `PRAGMA journal_mode=WAL` returns a row with the mode still in force
  (`delete`).
- `sqlite3_wal_hook` does nothing and returns NULL.
- `sqlite3_wal_checkpoint_v2` returns `SQLITE_OK` and leaves its two output
  numbers alone.

WebKit's `platform/sql` asks for WAL (`SQLiteDatabase::useWALJournalMode`):
it prepares and steps the pragma, gets its row, and carries on (the check of
the mode's name is debug-only). Its later checkpoint calls get `SQLITE_OK`.
That is exactly what happens in the browser today with the static library,
which is built with the same option.

## Starting: the library is missing or too old

`libsqlite3_link.a` opens `sqlite.library` (version 1 or later) before
`main()` and closes it after. If it can't, the program never reaches
`main()`: it shows "This program needs sqlite.library 1 or later in LIBS:"
(a requester when started from Workbench, a line in the Shell) and ends with
`RETURN_FAIL`. This is GFOpenLayout's arrangement
(`gfopenlayout_link_base.c`).

A program that wants its own requester, in its own words and at the moment
it chooses, defines `struct Library *SQLiteBase;` itself. That keeps the
automatic start-up out of the link, and the program calls
`sqlite3_link_open()` (1 or 0) and `sqlite3_link_close()` from
`<sqlite3_link.h>`. This is GFOffice's arrangement for gfopenlayout.library
(`ow_layout_open`).

A call added in a later library version answers 0 through the link library
when the library in `LIBS:` is older, so a program built against version 2
still starts on version 1.

## Tests

- `tools/gen-library.py --check`: the generated files are what the table
  says. The library's source fails to compile if its vector table and the
  generated calls disagree in number.
- `tests/host-libtest.sh`: `tests/libtest.c` on this computer. The link
  library's functions call the library's side as plain C, and that calls the
  real SQLite. Every split 64-bit value, double by pointer, callback cast,
  unpacked argument list and owner token runs as written. It can't show the
  registers or the Amiga layer.
- `tests/host-amigatest.sh`: the Amiga layer itself (`library/sqlite_amiga.c`:
  both VFSes, the `.lck` lock, seeks past the end, the pool, the semaphore
  mutexes) on this computer under SQLite built with the library's options,
  with `tests/hostamiga.h` standing in for exec and dos.library (a `Lock()`
  is a `flock()`, and `Seek()` refuses a place past the end as AmigaDOS
  does). Three programs run over it: `tests/locktest.c` (two writers, a
  reader waiting on another thread, a writer in another process, a process
  that dies in the middle of a write, an old lock drawer), `tests/baseprobe.c`
  (GFOpenBase's probe) and `tests/libtest.c` again.
- `build-library.sh`: the 68040 build, with the checks listed at its top.
- `tests/run-vamos.sh`: `SQLiteLibTestSoft` through the real library as
  68040 code under vamos (amitools), on a 16 KB stack so the library's own
  stack for a call is used. vamos runs one task, has no FPU, does not refuse
  a second exclusive `Lock()` and does not show one file handle another's
  writes, so the second task, the FPU path and two connections meeting are
  left to the two tests above and to an Amiga.
- `SQLiteLibTest` on an Amiga, twice in a row: adds the second task (its own
  connection and the first task's), the FPU path, the real lock, and the
  exit that closes a connection left open (the second run fails if the first
  left it locked).
