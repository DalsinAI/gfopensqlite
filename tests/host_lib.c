/* The library's side of sqlite.library on this computer, for
 * tests/host-libtest.sh: the calls tools/gen-library.py makes
 * (library/sqlite_lib_calls.h), with their arguments as plain C arguments
 * instead of registers, over the real SQLite. The link library's functions
 * reach them through tests/host_inline.h.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdarg.h>
#include "sqlite3.h"
#include "sqlite_own.h"

typedef void *APTR;
typedef unsigned int ULONG;
typedef int LONG;

#define REG(r, decl) decl
#define SQL_ENTRY

/* every call that would get a stack of its own is counted */
int host_deep_calls;

static LONG sql_run(LONG (*fn)(APTR), APTR arg)
{
    host_deep_calls++;
    fn(arg);
    return 0;
}

#include "sqlite_lib_calls.h"

/* the table's size, for the test's report */
int host_calls(void) { return SQL_CALLS; }
