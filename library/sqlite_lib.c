/* sqlite.library 1.0 (GFOpenSQLite): SQLite as a shared library.
 * docs/sqlite.library.md is the contract; the calls are SQLite's own C
 * functions behind the entry points tools/gen-library.py makes
 * (sqlite_lib_calls.h), in library/sqlite_lib.sfd's order.
 *   - SQLite is built in, with its files on dos.library and its memory
 *     and mutexes on exec (sqlite_amiga.c). It is started when the first
 *     program opens the library and shut down, its memory given back,
 *     when the last one closes it.
 *   - Any number of programs and tasks may call at once (SQLite's
 *     serialized mode on SignalSemaphores). Callers must be processes:
 *     dos.library does the file work on the caller's behalf.
 *   - A call that runs SQL gets a stack of its own when the caller has
 *     less than SQL_STACK_NEEDED free, so a 4 KB shell stack is enough.
 *     Callbacks are plain C calls on the calling task, on that stack.
 *   - Floating point is soft float through mathieeedoubbas/doubtrans,
 *     with fpuauto: the FPU when there is one. Doubles cross the library
 *     by pointer, so hard-float and soft-float programs both fit.
 *   - A program that ends without closing its connections has them closed
 *     when it detaches (sqlite_own.c).
 * The frame is GFOpenTransMem's library/tm_lib.c.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <stdarg.h>
#include <libraries/sqlite.h>
#include "sqlite3.h"
#include "sqlite_own.h"
#include "sqlite_amiga.h"

#define REG(r, decl) register decl __asm(#r)
#define SQL_ENTRY static
#define LIB_VERSION 1
#define LIB_REVISION 0
#define SQL_STACK_NEEDED 49152
#define SQL_STACK_SIZE 131072

struct SQLiteLibBase {
    struct Library lib;
    BPTR seglist;
};

struct ExecBase *SysBase;
struct DosLibrary *DOSBase;
struct Library *MathIeeeDoubBasBase, *MathIeeeDoubTransBase;
struct Library *MathIeeeSingBasBase, *MathIeeeSingTransBase;

static struct SignalSemaphore lib_sem;  /* the maths libraries, SQLite's start and end */
static int sqlite_ready;

extern LONG sql_onstack(LONG (*fn)(APTR), APTR arg, struct StackSwapStruct *sss);
extern ULONG sql_fpcr_get(void);
extern void sql_fpcr_set(ULONG);

/* Run as a program, the library does nothing. This must stay the first code
 * in the file (-fno-toplevel-reorder). */
int start(void) { return -1; }

static const char lib_name[] = SQLITELIB_NAME;
static const char lib_id[] =
    "sqlite.library 1.0 (18.11.2026) SQLite " SQLITE_VERSION ", GFOpenSQLite, AmiCradle Team\r\n";
static const char lib_ver[] __attribute__((used)) =
    "$VER: sqlite.library 1.0 (18.11.2026) SQLite " SQLITE_VERSION ", GFOpenSQLite, AmiCradle Team";

/* ---- A call on enough stack -------------------------------------------- */

/* fn(arg), on a stack of its own when the caller's is small. Nonzero when
 * that stack could not be had (the call was not made). A caller on a stack
 * exec does not know (outside tc_SPLower..tc_SPUpper) is taken at its word,
 * as it would be with SQLite linked in. */
static LONG sql_run(LONG (*fn)(APTR), APTR arg)
{
    struct Task *me = FindTask(0);
    struct StackSwapStruct sss;
    ULONG here = (ULONG)&sss;
    UBYTE *stack;
    if (here <= (ULONG)me->tc_SPLower || here > (ULONG)me->tc_SPUpper ||
        here - (ULONG)me->tc_SPLower >= SQL_STACK_NEEDED) {
        fn(arg);
        return 0;
    }
    if (!(stack = (UBYTE *)AllocMem(SQL_STACK_SIZE, MEMF_ANY))) return 1;
    sss.stk_Lower = stack;
    sss.stk_Upper = (ULONG)(stack + SQL_STACK_SIZE);
    sss.stk_Pointer = (APTR)(stack + SQL_STACK_SIZE);
    sql_onstack(fn, arg, &sss);
    FreeMem(stack, SQL_STACK_SIZE);
    return 0;
}

/* ---- The calls --------------------------------------------------------- */

#include "sqlite_lib_calls.h"

/* ---- Library housekeeping ---------------------------------------------- */

static struct Library *lib_init(REG(d0, struct SQLiteLibBase *base), REG(a0, BPTR seglist),
                                REG(a6, struct ExecBase *sys));
static struct Library *lib_open(REG(a6, struct SQLiteLibBase *base));
static BPTR lib_close(REG(a6, struct SQLiteLibBase *base));
static BPTR lib_expunge(REG(a6, struct SQLiteLibBase *base));
static ULONG lib_null(void);

static const APTR lib_vectors[] = {
    (APTR)lib_open, (APTR)lib_close, (APTR)lib_expunge, (APTR)lib_null,
    SQL_VECTORS,
    (APTR)-1
};

/* the table and the generated calls agree: 4 housekeeping vectors, the calls, the end mark */
typedef char sql_vector_count[sizeof lib_vectors / sizeof lib_vectors[0] == SQL_CALLS + 5 ? 1 : -1];
typedef char sql_last_bias[SQL_LAST_BIAS == 30 + 6 * (SQL_CALLS - 1) ? 1 : -1];

static const struct {
    ULONG size;
    const APTR *vectors;
    APTR data;
    APTR init;
} lib_inittable = {
    sizeof(struct SQLiteLibBase), lib_vectors, NULL, (APTR)lib_init
};

const struct Resident lib_romtag = {
    RTC_MATCHWORD, (struct Resident *)&lib_romtag, (APTR)(&lib_romtag + 1),
    RTF_AUTOINIT, LIB_VERSION, NT_LIBRARY, 0,
    (char *)lib_name, (char *)lib_id, (APTR)&lib_inittable
};

static struct Library *lib_init(REG(d0, struct SQLiteLibBase *base), REG(a0, BPTR seglist),
                                REG(a6, struct ExecBase *sys))
{
    SysBase = sys;
    base->seglist = seglist;
    base->lib.lib_Node.ln_Type = NT_LIBRARY;
    base->lib.lib_Node.ln_Name = (char *)lib_name;
    base->lib.lib_Flags = LIBF_SUMUSED | LIBF_CHANGED;
    base->lib.lib_Version = LIB_VERSION;
    base->lib.lib_Revision = LIB_REVISION;
    base->lib.lib_IdString = (APTR)lib_id;
    InitSemaphore(&lib_sem);
    sql_amiga_init();
    if (!(DOSBase = (struct DosLibrary *)OpenLibrary((CONST_STRPTR)"dos.library", 39))) {
        FreeMem((UBYTE *)base - base->lib.lib_NegSize, base->lib.lib_NegSize + base->lib.lib_PosSize);
        return 0;
    }
    return &base->lib;
}

static void close_maths(void)
{
    if (MathIeeeSingTransBase) CloseLibrary(MathIeeeSingTransBase);
    if (MathIeeeSingBasBase) CloseLibrary(MathIeeeSingBasBase);
    if (MathIeeeDoubTransBase) CloseLibrary(MathIeeeDoubTransBase);
    if (MathIeeeDoubBasBase) CloseLibrary(MathIeeeDoubBasBase);
    MathIeeeDoubBasBase = MathIeeeDoubTransBase = MathIeeeSingBasBase = MathIeeeSingTransBase = 0;
}

/* The maths libraries set the FPU's rounding precision for the task that
 * opens them (mathieeesingbas: single). That task is a program's, which may
 * be hard float and do doubles on the FPU itself, so its setting is put
 * back. Our own FPU code (fpuauto) rounds by instruction, not by setting. */
static int open_maths(void)
{
    int fpu = (SysBase->AttnFlags & AFF_68881) != 0;
    ULONG fpcr = fpu ? sql_fpcr_get() : 0;
    MathIeeeDoubBasBase = OpenLibrary((CONST_STRPTR)"mathieeedoubbas.library", 37);
    MathIeeeDoubTransBase = OpenLibrary((CONST_STRPTR)"mathieeedoubtrans.library", 37);
    MathIeeeSingBasBase = OpenLibrary((CONST_STRPTR)"mathieeesingbas.library", 37);
    MathIeeeSingTransBase = OpenLibrary((CONST_STRPTR)"mathieeesingtrans.library", 37);
    if (fpu) sql_fpcr_set(fpcr);
    if (MathIeeeDoubBasBase && MathIeeeDoubTransBase && MathIeeeSingBasBase && MathIeeeSingTransBase)
        return 1;
    close_maths();
    return 0;
}

static LONG do_start(APTR v)
{
    int *rc = (int *)v;
    *rc = sql_amiga_start();
    if (*rc == SQLITE_OK) *rc = sqlite3_initialize();
    return 0;
}

static LONG do_end(APTR v)
{
    (void)v;
    sqlite3_shutdown();
    return 0;
}

/* The maths libraries (the transcendental ones may have to be loaded from
 * LIBS:) and SQLite are started at the first open, not in lib_init. */
static struct Library *lib_open(REG(a6, struct SQLiteLibBase *base))
{
    struct Library *r = &base->lib;
    base->lib.lib_OpenCnt++;
    base->lib.lib_Flags &= ~LIBF_DELEXP;
    ObtainSemaphore(&lib_sem);
    if (!MathIeeeDoubBasBase && !open_maths()) r = 0;
    if (r && !sqlite_ready) {
        int rc = SQLITE_NOMEM;
        if (sql_run(do_start, &rc) == 0 && rc == SQLITE_OK) sqlite_ready = 1;
        else {
            sql_amiga_end();
            r = 0;
        }
    }
    if (!r) base->lib.lib_OpenCnt--;
    ReleaseSemaphore(&lib_sem);
    return r;
}

/* The last program out shuts SQLite down: its memory pool goes, and with
 * it anything a program allocated and never freed. */
static BPTR lib_close(REG(a6, struct SQLiteLibBase *base))
{
    ObtainSemaphore(&lib_sem);
    if (base->lib.lib_OpenCnt) base->lib.lib_OpenCnt--;
    if (base->lib.lib_OpenCnt == 0 && sqlite_ready) {
        if (sql_run(do_end, 0) != 0) do_end(0);
        sql_own_reset();
        sql_amiga_end();
        sqlite_ready = 0;
    }
    ReleaseSemaphore(&lib_sem);
    if (base->lib.lib_OpenCnt == 0 && (base->lib.lib_Flags & LIBF_DELEXP))
        return lib_expunge(base);
    return 0;
}

/* Called with task switching off, so it must not wait: when a close is in
 * the middle of shutting SQLite down (it holds lib_sem), go later. */
static BPTR lib_expunge(REG(a6, struct SQLiteLibBase *base))
{
    BPTR seglist;
    if (base->lib.lib_OpenCnt || !AttemptSemaphore(&lib_sem)) {
        base->lib.lib_Flags |= LIBF_DELEXP;
        return 0;
    }
    ReleaseSemaphore(&lib_sem);
    seglist = base->seglist;
    Remove(&base->lib.lib_Node);
    close_maths();
    CloseLibrary((struct Library *)DOSBase);
    FreeMem((UBYTE *)base - base->lib.lib_NegSize,
            base->lib.lib_NegSize + base->lib.lib_PosSize);
    return seglist;
}

static ULONG lib_null(void) { return 0; }
