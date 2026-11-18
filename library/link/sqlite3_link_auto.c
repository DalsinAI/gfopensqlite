/* libsqlite3_link.a's SQLiteBase: sqlite.library, opened before main() and
 * closed after it. Without the library the program stops at once with
 * RETURN_FAIL and says why (a requester from Workbench, a line in the
 * Shell): a call through no library would crash. This file comes in only
 * when the program has no SQLiteBase of its own; one that has calls
 * sqlite3_link_open() itself (include/sqlite3_link.h). As GFOpenLayout's
 * library/link/gfopenlayout_link_base.c.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <inline/macros.h>
#include <sqlite3_link.h>

struct Library *SQLiteBase;

extern void exit(int);

static void __attribute__((constructor)) sqlite3_link_auto_open(void)
{
    static const char why[] = "This program needs " SQLITE3_LINK_NEEDS "\n";
    struct Process *me = (struct Process *)FindTask(NULL);
    struct Library *lib;
    if (sqlite3_link_open()) return;
    if (me->pr_Task.tc_Node.ln_Type == NT_PROCESS && !me->pr_CLI) {
        /* from Workbench: nobody reads a Shell */
        if ((lib = OpenLibrary((CONST_STRPTR)"intuition.library", 37))) {
            static struct EasyStruct es = {
                sizeof(struct EasyStruct), 0, (UBYTE *)"SQLite",
                (UBYTE *)"This program needs " SQLITE3_LINK_NEEDS, (UBYTE *)"OK"
            };
            LP4(0x24c, LONG, EasyRequestArgs, struct Window *, NULL, a0, struct EasyStruct *, &es, a1,
                ULONG *, NULL, a2, APTR, NULL, a3, , lib);
            CloseLibrary(lib);
        }
    } else if ((lib = OpenLibrary((CONST_STRPTR)"dos.library", 36))) {
        BPTR out = LP0(0x3c, BPTR, Output, , lib);
        if (out) LP3(0x30, LONG, Write, BPTR, out, d1, APTR, (APTR)why, d2, LONG, (LONG)sizeof(why) - 1, d3, , lib);
        CloseLibrary(lib);
    }
    exit(RETURN_FAIL);
}

static void __attribute__((destructor)) sqlite3_link_auto_close(void)
{
    sqlite3_link_close();
}
