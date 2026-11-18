#!/usr/bin/env python3
"""sqlite.library's calls, made from one table.

The table below is the SQLite C API a program reaches through sqlite.library:
what GFOpenBase, GFOffice's Base and Calc, the browser's WebKit
(platform/sql) and GFOpenTransMem call, with the calls that go with them
(docs/sqlite.library.md has the survey). Each line is the C prototype as
sqlite3.h has it, and the library call is made from it:

  sqlite3_bind_int64  ->  SQL_BindInt64, its arguments in registers:
                          pointers in a0 to a3, then the rest in d0 to d7
  a 64-bit argument   ->  two registers, high half then low half
  a 64-bit result     ->  written through a pointer the program passes
  a double argument   ->  passed by pointer; a double result likewise
                          (a program may be hard float, the library is
                          soft float: neither d0/d1 nor fp0 suits both)
  a function pointer  ->  an APTR; the library calls it as a plain C
                          function on the caller's task

From the table this writes, whole:
  library/sqlite_lib.sfd         the calls, in library order
  library/sqlite_lib_calls.h     the library's side: each call with its
                                 arguments from their registers, and
                                 SQL_VECTORS for the library's table
  include/inline/sqlite.h        the program's inline calls
  library/link/sqlite3_link.c    the C API through the library: a program
                                 that calls sqlite3_open() and the rest
                                 links this (libsqlite3_link.a) where it
                                 linked libsqlite3.a
  tests/host_inline.h            the same calls as plain C calls, for the
                                 test that runs the link library and the
                                 library's side on this computer
  tests/host_rename.h            sqlite3_x -> lnk_sqlite3_x for that test

Never reorder the table or take a line out: only add at the end, with the
next version number. --check says whether the files are as this would write
them (build-library.sh runs that).
MIT, Copyright (c) 2026 AmiCradle Team."""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FIRST_BIAS = 30
MAX_ARGS = 10               # inline/macros.h has LP0 to LP10 with and without a result

# version, flags, prototype. Flags:
#   deep       the call can run SQL: it gets a stack of its own when the
#              caller's has under SQL_STACK_NEEDED free (sqlite_lib.c)
#   via=NAME   the library's side calls NAME (library/sqlite_own.c), not
#              the SQLite function
#   owner      the program's owner token goes with the call (after the C
#              arguments): the connection is closed for a program that
#              ends without closing it
#   lib        a call of the library alone: no sqlite3_ function of that
#              name in the link library's generated part
TABLE = """
1 lib,via=sql_attach        void *SQL_Attach(void)
1 lib,deep,via=sql_detach   void SQL_Detach(void *owner)
1 lib,via=sql_db_config     int SQL_DbConfig(sqlite3 *db, int op, void *p, int a, int b)
1 -            const char *sqlite3_libversion(void)
1 -            const char *sqlite3_sourceid(void)
1 -            int sqlite3_libversion_number(void)
1 -            int sqlite3_threadsafe(void)
1 deep,owner,via=sql_open     int sqlite3_open(const char *filename, sqlite3 **ppDb)
1 deep,owner,via=sql_open_v2  int sqlite3_open_v2(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs)
1 deep,via=sql_close        int sqlite3_close(sqlite3 *db)
1 deep,via=sql_close_v2     int sqlite3_close_v2(sqlite3 *db)
1 deep         int sqlite3_exec(sqlite3 *db, const char *sql, int (*callback)(void*,int,char**,char**), void *arg, char **errmsg)
1 -            int sqlite3_extended_result_codes(sqlite3 *db, int onoff)
1 -            sqlite3_int64 sqlite3_last_insert_rowid(sqlite3 *db)
1 -            void sqlite3_set_last_insert_rowid(sqlite3 *db, sqlite3_int64 rowid)
1 -            int sqlite3_changes(sqlite3 *db)
1 -            sqlite3_int64 sqlite3_changes64(sqlite3 *db)
1 -            int sqlite3_total_changes(sqlite3 *db)
1 -            sqlite3_int64 sqlite3_total_changes64(sqlite3 *db)
1 -            void sqlite3_interrupt(sqlite3 *db)
1 -            int sqlite3_is_interrupted(sqlite3 *db)
1 -            int sqlite3_complete(const char *sql)
1 -            int sqlite3_busy_handler(sqlite3 *db, int (*xBusy)(void*,int), void *arg)
1 -            int sqlite3_busy_timeout(sqlite3 *db, int ms)
1 -            void *sqlite3_malloc(int n)
1 -            void *sqlite3_malloc64(sqlite3_uint64 n)
1 -            void *sqlite3_realloc(void *p, int n)
1 -            void *sqlite3_realloc64(void *p, sqlite3_uint64 n)
1 -            void sqlite3_free(void *p)
1 -            sqlite3_uint64 sqlite3_msize(void *p)
1 -            sqlite3_int64 sqlite3_memory_used(void)
1 -            sqlite3_int64 sqlite3_memory_highwater(int resetFlag)
1 -            void sqlite3_randomness(int N, void *P)
1 -            int sqlite3_set_authorizer(sqlite3 *db, int (*xAuth)(void*,int,const char*,const char*,const char*,const char*), void *pUserData)
1 -            void sqlite3_progress_handler(sqlite3 *db, int nOps, int (*xProgress)(void*), void *pArg)
1 -            int sqlite3_errcode(sqlite3 *db)
1 -            int sqlite3_extended_errcode(sqlite3 *db)
1 -            const char *sqlite3_errmsg(sqlite3 *db)
1 -            const char *sqlite3_errstr(int rc)
1 -            int sqlite3_error_offset(sqlite3 *db)
1 -            int sqlite3_system_errno(sqlite3 *db)
1 -            int sqlite3_limit(sqlite3 *db, int id, int newVal)
1 deep         int sqlite3_prepare_v2(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail)
1 deep         int sqlite3_prepare_v3(sqlite3 *db, const char *zSql, int nByte, unsigned int prepFlags, sqlite3_stmt **ppStmt, const char **pzTail)
1 -            const char *sqlite3_sql(sqlite3_stmt *pStmt)
1 -            char *sqlite3_expanded_sql(sqlite3_stmt *pStmt)
1 -            int sqlite3_stmt_readonly(sqlite3_stmt *pStmt)
1 -            int sqlite3_stmt_isexplain(sqlite3_stmt *pStmt)
1 -            int sqlite3_stmt_busy(sqlite3_stmt *pStmt)
1 -            int sqlite3_stmt_status(sqlite3_stmt *pStmt, int op, int resetFlg)
1 -            int sqlite3_bind_blob(sqlite3_stmt *pStmt, int idx, const void *data, int n, void (*xDel)(void*))
1 -            int sqlite3_bind_blob64(sqlite3_stmt *pStmt, int idx, const void *data, sqlite3_uint64 n, void (*xDel)(void*))
1 -            int sqlite3_bind_double(sqlite3_stmt *pStmt, int idx, double value)
1 -            int sqlite3_bind_int(sqlite3_stmt *pStmt, int idx, int value)
1 -            int sqlite3_bind_int64(sqlite3_stmt *pStmt, int idx, sqlite3_int64 value)
1 -            int sqlite3_bind_null(sqlite3_stmt *pStmt, int idx)
1 -            int sqlite3_bind_text(sqlite3_stmt *pStmt, int idx, const char *data, int n, void (*xDel)(void*))
1 -            int sqlite3_bind_text64(sqlite3_stmt *pStmt, int idx, const char *data, sqlite3_uint64 n, void (*xDel)(void*), unsigned char encoding)
1 -            int sqlite3_bind_value(sqlite3_stmt *pStmt, int idx, const sqlite3_value *value)
1 -            int sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, int idx, int n)
1 -            int sqlite3_bind_parameter_count(sqlite3_stmt *pStmt)
1 -            const char *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, int idx)
1 -            int sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, const char *zName)
1 -            int sqlite3_clear_bindings(sqlite3_stmt *pStmt)
1 -            int sqlite3_column_count(sqlite3_stmt *pStmt)
1 -            const char *sqlite3_column_name(sqlite3_stmt *pStmt, int N)
1 -            const char *sqlite3_column_decltype(sqlite3_stmt *pStmt, int N)
1 deep         int sqlite3_step(sqlite3_stmt *pStmt)
1 -            int sqlite3_data_count(sqlite3_stmt *pStmt)
1 -            const void *sqlite3_column_blob(sqlite3_stmt *pStmt, int iCol)
1 -            double sqlite3_column_double(sqlite3_stmt *pStmt, int iCol)
1 -            int sqlite3_column_int(sqlite3_stmt *pStmt, int iCol)
1 -            sqlite3_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, int iCol)
1 -            const unsigned char *sqlite3_column_text(sqlite3_stmt *pStmt, int iCol)
1 -            sqlite3_value *sqlite3_column_value(sqlite3_stmt *pStmt, int iCol)
1 -            int sqlite3_column_bytes(sqlite3_stmt *pStmt, int iCol)
1 -            int sqlite3_column_type(sqlite3_stmt *pStmt, int iCol)
1 deep         int sqlite3_finalize(sqlite3_stmt *pStmt)
1 deep         int sqlite3_reset(sqlite3_stmt *pStmt)
1 -            int sqlite3_create_function(sqlite3 *db, const char *zFunctionName, int nArg, int eTextRep, void *pApp, void (*xFunc)(sqlite3_context*,int,sqlite3_value**), void (*xStep)(sqlite3_context*,int,sqlite3_value**), void (*xFinal)(sqlite3_context*))
1 -            int sqlite3_create_function_v2(sqlite3 *db, const char *zFunctionName, int nArg, int eTextRep, void *pApp, void (*xFunc)(sqlite3_context*,int,sqlite3_value**), void (*xStep)(sqlite3_context*,int,sqlite3_value**), void (*xFinal)(sqlite3_context*), void (*xDestroy)(void*))
1 -            const void *sqlite3_value_blob(sqlite3_value *pVal)
1 -            double sqlite3_value_double(sqlite3_value *pVal)
1 -            int sqlite3_value_int(sqlite3_value *pVal)
1 -            sqlite3_int64 sqlite3_value_int64(sqlite3_value *pVal)
1 -            const unsigned char *sqlite3_value_text(sqlite3_value *pVal)
1 -            int sqlite3_value_bytes(sqlite3_value *pVal)
1 -            int sqlite3_value_type(sqlite3_value *pVal)
1 -            int sqlite3_value_numeric_type(sqlite3_value *pVal)
1 -            sqlite3_value *sqlite3_value_dup(const sqlite3_value *pVal)
1 -            void sqlite3_value_free(sqlite3_value *pVal)
1 -            void *sqlite3_aggregate_context(sqlite3_context *ctx, int nBytes)
1 -            void *sqlite3_user_data(sqlite3_context *ctx)
1 -            sqlite3 *sqlite3_context_db_handle(sqlite3_context *ctx)
1 -            void sqlite3_result_blob(sqlite3_context *ctx, const void *data, int n, void (*xDel)(void*))
1 -            void sqlite3_result_double(sqlite3_context *ctx, double value)
1 -            void sqlite3_result_error(sqlite3_context *ctx, const char *msg, int n)
1 -            void sqlite3_result_error_toobig(sqlite3_context *ctx)
1 -            void sqlite3_result_error_nomem(sqlite3_context *ctx)
1 -            void sqlite3_result_error_code(sqlite3_context *ctx, int code)
1 -            void sqlite3_result_int(sqlite3_context *ctx, int value)
1 -            void sqlite3_result_int64(sqlite3_context *ctx, sqlite3_int64 value)
1 -            void sqlite3_result_null(sqlite3_context *ctx)
1 -            void sqlite3_result_text(sqlite3_context *ctx, const char *data, int n, void (*xDel)(void*))
1 -            void sqlite3_result_value(sqlite3_context *ctx, sqlite3_value *value)
1 -            void sqlite3_result_zeroblob(sqlite3_context *ctx, int n)
1 -            int sqlite3_create_collation(sqlite3 *db, const char *zName, int eTextRep, void *pArg, int (*xCompare)(void*,int,const void*,int,const void*))
1 -            int sqlite3_create_collation_v2(sqlite3 *db, const char *zName, int eTextRep, void *pArg, int (*xCompare)(void*,int,const void*,int,const void*), void (*xDestroy)(void*))
1 -            int sqlite3_get_autocommit(sqlite3 *db)
1 -            int sqlite3_txn_state(sqlite3 *db, const char *zSchema)
1 -            sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt)
1 -            const char *sqlite3_db_filename(sqlite3 *db, const char *zDbName)
1 -            int sqlite3_db_readonly(sqlite3 *db, const char *zDbName)
1 -            sqlite3_stmt *sqlite3_next_stmt(sqlite3 *db, sqlite3_stmt *pStmt)
1 -            void *sqlite3_commit_hook(sqlite3 *db, int (*xCallback)(void*), void *pArg)
1 -            void *sqlite3_rollback_hook(sqlite3 *db, void (*xCallback)(void*), void *pArg)
1 -            void *sqlite3_update_hook(sqlite3 *db, void (*xCallback)(void*,int,const char*,const char*,sqlite3_int64), void *pArg)
1 -            int sqlite3_trace_v2(sqlite3 *db, unsigned int uMask, int (*xCallback)(unsigned int,void*,void*,void*), void *pCtx)
1 deep         int sqlite3_db_release_memory(sqlite3 *db)
1 deep         int sqlite3_db_cacheflush(sqlite3 *db)
1 deep         int sqlite3_file_control(sqlite3 *db, const char *zDbName, int op, void *pArg)
1 -            void *sqlite3_wal_hook(sqlite3 *db, int (*xCallback)(void*,sqlite3*,const char*,int), void *pArg)
1 deep         int sqlite3_wal_checkpoint_v2(sqlite3 *db, const char *zDb, int eMode, int *pnLog, int *pnCkpt)
1 -            sqlite3_str *sqlite3_str_new(sqlite3 *db)
1 -            char *sqlite3_str_finish(sqlite3_str *p)
1 -            void sqlite3_str_vappendf(sqlite3_str *p, const char *zFormat, va_list ap)
1 -            void sqlite3_str_append(sqlite3_str *p, const char *zIn, int N)
1 -            void sqlite3_str_appendall(sqlite3_str *p, const char *zIn)
1 -            void sqlite3_str_appendchar(sqlite3_str *p, int N, char C)
1 -            void sqlite3_str_reset(sqlite3_str *p)
1 -            int sqlite3_str_errcode(sqlite3_str *p)
1 -            int sqlite3_str_length(sqlite3_str *p)
1 -            char *sqlite3_str_value(sqlite3_str *p)
1 -            char *sqlite3_vmprintf(const char *zFormat, va_list ap)
1 -            char *sqlite3_vsnprintf(int n, char *zBuf, const char *zFormat, va_list ap)
1 -            int sqlite3_stricmp(const char *a, const char *b)
1 -            int sqlite3_strnicmp(const char *a, const char *b, int n)
1 -            int sqlite3_strglob(const char *zGlob, const char *zStr)
1 -            int sqlite3_strlike(const char *zGlob, const char *zStr, unsigned int cEsc)
1 -            int sqlite3_sleep(int ms)
1 -            sqlite3_vfs *sqlite3_vfs_find(const char *zVfsName)
"""

WIDE = {"sqlite3_int64", "sqlite3_uint64", "sqlite_int64", "sqlite_uint64"}


class Param:
    """One C argument: its type, its name, and (a function pointer) its cast."""

    def __init__(self, text, call):
        text = " ".join(text.split())
        m = re.match(r"(.*?)\(\s*\*\s*(\w+)\s*\)\s*(\(.*\))$", text)
        if m:
            self.kind, self.name = "fn", m.group(2)
            self.type = f"{m.group(1).strip()} (*)" + m.group(3)
            self.decl = f"{m.group(1).strip()} (*{self.name}){m.group(3)}"
            return
        m = re.match(r"(.*?[\s\*])(\w+)$", text)
        if not m:
            sys.exit(f"gen-library: {call}: an argument with no name: {text}")
        self.type, self.name = m.group(1).strip(), m.group(2)
        self.decl = f"{self.type} {self.name}".replace("* ", "*")
        if self.type in WIDE:
            self.kind = "wide"
        elif self.type == "double":
            self.kind = "double"
        elif "*" in self.type or self.type == "va_list":
            self.kind = "ptr"
        else:
            self.kind = "int"


def split_args(s):
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch == "," and depth == 0:
            out.append(cur)
            cur = ""
            continue
        depth += ch == "("
        depth -= ch == ")"
        cur += ch
    if cur.strip():
        out.append(cur)
    return out


def lib_name(c):
    """sqlite3_bind_int64 -> SQL_BindInt64"""
    if c.startswith("SQL_"):
        return c
    return "SQL_" + "".join(w[:1].upper() + w[1:] for w in c[len("sqlite3_"):].split("_"))


class Call:
    def __init__(self, version, flags, proto):
        self.version = int(version)
        self.flags = {}
        for f in flags.split(","):
            if f != "-":
                k, _, v = f.partition("=")
                self.flags[k] = v or True
        m = re.match(r"(.*?[\s\*])(\w+)\((.*)\)$", proto.strip())
        if not m:
            sys.exit(f"gen-library: not a prototype: {proto}")
        self.ret = m.group(1).strip()
        self.name = m.group(2)
        self.lib = lib_name(self.name)
        args = m.group(3).strip()
        self.params = [] if args == "void" else [Param(a, self.name) for a in split_args(args)]
        # the library call's own arguments: (type, name, the C argument they came from)
        self.entry = []
        for p in self.params:
            if p.kind == "wide":
                self.entry += [("ULONG", p.name + "_hi", p), ("ULONG", p.name + "_lo", p)]
            elif p.kind == "double":
                self.entry.append(("const double *", p.name + "_p", p))
            elif p.kind == "fn":
                self.entry.append(("APTR", p.name, p))
            else:
                self.entry.append((p.type, p.name, p))
        if "owner" in self.flags:
            self.entry.append(("APTR", "owner", None))
        self.by_pointer = self.ret in WIDE or self.ret == "double"
        if self.by_pointer:
            self.entry.append((self.ret + " *", "lib_result", None))
        self.entry_ret = "void" if self.by_pointer else self.ret
        if len(self.entry) > MAX_ARGS:
            sys.exit(f"gen-library: {self.name} has more arguments than inline/macros.h takes")
        a, d = ["a0", "a1", "a2", "a3"], [f"d{i}" for i in range(8)]
        self.regs = []
        for t, _, _ in self.entry:
            pool = a if ("*" in t or t in ("APTR", "va_list")) and a else d
            if not pool:
                sys.exit(f"gen-library: {self.name} has more arguments than registers")
            self.regs.append(pool.pop(0))

    def target(self):
        return self.flags.get("via") or self.name

    def c_args(self, prefix=""):
        """the C call's arguments, from the library call's (prefix: 'x->' in a deep call)"""
        out = []
        for p in self.params:
            n = prefix + p.name
            if p.kind == "wide":
                out.append(f"({p.type})(((sqlite3_uint64){n}_hi << 32) | {n}_lo)")
            elif p.kind == "double":
                out.append(f"*{n}_p")
            elif p.kind == "fn":
                out.append(f"({p.type}){n}")
            else:
                out.append(n)
        if "owner" in self.flags:
            out.append(prefix + "owner")
        return ", ".join(out)

    def lib_args(self):
        """the library call's arguments, from the C call's (the link library's side)"""
        out = []
        for p in self.params:
            if p.kind == "wide":
                out += [f"(ULONG)((sqlite3_uint64){p.name} >> 32)", f"(ULONG){p.name}"]
            elif p.kind == "double":
                out.append(f"&{p.name}")
            elif p.kind == "fn":
                out.append(f"(APTR){p.name}")
            else:
                out.append(p.name)
        if "owner" in self.flags:
            out.append("sqlite3_link_owner")
        if self.by_pointer:
            out.append("&lib_result")
        return ", ".join(out)


def decl(t, n):
    return f"{t}{n}" if t.endswith("*") else f"{t} {n}"


def calls():
    out = []
    for line in TABLE.strip().splitlines():
        version, flags, proto = line.split(None, 2)
        out.append(Call(version, flags, proto))
    seen = set()
    for c in out:
        if c.lib in seen:
            sys.exit(f"gen-library: {c.lib} twice")
        seen.add(c.lib)
    return out


def default_result(c):
    """what a deep call answers when it could not get its stack"""
    if c.entry_ret == "int":
        return "SQLITE_NOMEM"
    return "0"


def generate():
    cs = calls()
    sfd = ["==id $Id: sqlite_lib.sfd 1.0 (18.11.2026) $",
           '* "sqlite.library": made by tools/gen-library.py, don\'t edit',
           "==base _SQLiteBase", "==basetype struct Library *", "==libname sqlite.library",
           f"==bias {FIRST_BIAS}", "==public", "==include <exec/types.h>", "==include <stdarg.h>",
           "==include <sqlite3.h>"]
    inline, host, lib, link, vectors, rename = [], [], [], [], [], []
    bias = FIRST_BIAS
    version = 0
    for c in cs:
        if c.version != version:
            if c.version < version:
                sys.exit(f"gen-library: {c.name}: version {c.version} after version {version}")
            version = c.version
            sfd.append(f"==version {version}" if version > 1 else "* version 1")
            inline.append(f"/* version {version} */")
        L = c.lib
        sfd.append(f"{decl(c.entry_ret, L)}({', '.join(decl(t, n) for t, n, _ in c.entry)})({','.join(c.regs)})")
        # the program's inline call
        names = ",".join(n for _, n, _ in c.entry)
        void = c.entry_ret == "void"
        mac = [f"0x{bias:x}"]
        if not void:
            mac.append(c.entry_ret)
        mac.append(L)
        for (t, n, _), r in zip(c.entry, c.regs):
            mac += [t, f"({n})", r]
        mac += ["", "SQLITE_BASE_NAME"]
        inline.append(f"#define {L}({names}) LP{len(c.entry)}{'NR' if void else ''}({', '.join(mac)})")
        # the same call as a plain C call (the host test)
        hargs = ", ".join(decl(t, n) for t, n, _ in c.entry) or "void"
        host.append(f"{c.entry_ret} L_{L}({hargs});\n#define {L}({names}) L_{L}({names})")
        # the library's side
        regs = ", ".join(f"REG({r}, {decl(t, n)})" for (t, n, _), r in zip(c.entry, c.regs)) or "void"
        if "deep" in c.flags:
            fields = "".join(f" {decl(t, n)};" for t, n, _ in c.entry)
            res = "" if void else f" {decl(c.entry_ret, 'r')};"
            call = f"{c.target()}({c.c_args('x->')})"
            if c.by_pointer:
                call = f"*x->lib_result = {call}"
            elif not void:
                call = f"x->r = {call}"
            sets = "".join(f" x.{n} = {n};" for _, n, _ in c.entry)
            if not void:
                sets += f" x.r = {default_result(c)};"
            lib.append(
                f"struct x_{L} {{{fields}{res} }};\n"
                f"static LONG d_{L}(APTR v)\n{{\n    struct x_{L} *x = (struct x_{L} *)v;\n    {call};\n    return 0;\n}}\n"
                f"SQL_ENTRY {c.entry_ret} L_{L}({regs})\n{{\n    struct x_{L} x;\n   {sets}\n"
                + (f"    if (sql_run(d_{L}, &x)) d_{L}(&x);\n" if void else f"    sql_run(d_{L}, &x);\n    return x.r;\n") + "}")
        else:
            call = f"{c.target()}({c.c_args()})"
            if c.by_pointer:
                body = f"*lib_result = {call};"
            elif void:
                body = f"{call};"
            else:
                body = f"return {call};"
            lib.append(f"SQL_ENTRY {c.entry_ret} L_{L}({regs})\n{{\n    {body}\n}}")
        vectors.append(f"(APTR)L_{L}")
        # the program's side: the C call through the library
        if "lib" not in c.flags:
            cargs = ", ".join(p.decl for p in c.params) or "void"
            guard = ""
            if c.version > 1:
                guard = (f"    if (SQLiteBase->lib_Version < {c.version}) "
                         f"return{'' if c.ret == 'void' else ' 0'};\n")
            if c.by_pointer:
                body = f"    {c.ret} lib_result = 0;\n{guard}    {L}({c.lib_args()});\n    return lib_result;"
            elif c.ret == "void":
                body = f"{guard}    {L}({c.lib_args()});"
            else:
                body = f"{guard}    return {L}({c.lib_args()});"
            link.append(f"{decl(c.ret, c.name)}({cargs})\n{{\n{body}\n}}")
            rename.append(c.name)
        bias += 6
    last = bias - 6
    head = "/* Made by tools/gen-library.py from its table: don't edit."
    v = ", \\\n    ".join(", ".join(vectors[i:i + 3]) for i in range(0, len(vectors), 3))
    libh = (f"{head} sqlite.library's calls,\n"
            " * each the C call with its arguments from their registers. A call that can\n"
            " * run SQL goes through sql_run (a stack of its own when the caller's is small).\n"            " * When that stack can't be had the call answers its failure value; a call\n"
            " * with no answer to give (SQL_Detach) runs on the caller's stack instead.\n"
            " * MIT, Copyright (c) 2026 AmiCradle Team. */\n\n"
            + "\n\n".join(lib)
            + "\n\n/* the last call's bias and the number of calls: checked against the table */\n"
            + f"#define SQL_LAST_BIAS 0x{last:x}\n#define SQL_CALLS {len(cs)}\n"
            + "\n#define SQL_VECTORS \\\n    " + v + "\n")
    inl = (f"{head} sqlite.library's calls for a\n"
           " * program that calls the library itself (most link libsqlite3_link.a and call\n"
           " * sqlite3_open() and the rest as before). A 64-bit argument is two (high half,\n"
           " * low half); a 64-bit or double result comes through the last argument.\n"
           " * MIT, Copyright (c) 2026 AmiCradle Team. */\n"
           "#ifndef INLINE_SQLITE_H\n#define INLINE_SQLITE_H\n"
           "#ifndef __INLINE_MACROS_H\n#include <inline/macros.h>\n#endif\n"
           "#ifndef SQLITE_BASE_NAME\n#define SQLITE_BASE_NAME SQLiteBase\n#endif\n\n"
           + "\n".join(inline) + "\n\n#endif\n")
    hosth = (f"{head} sqlite.library's calls as\n"
             " * plain C calls into the library's side, for the test that runs the link\n"
             " * library on this computer (tests/host-libtest.sh).\n"
             " * MIT, Copyright (c) 2026 AmiCradle Team. */\n"
             "#ifndef SQLITE_HOST_INLINE_H\n#define SQLITE_HOST_INLINE_H\n\n"
             + "\n".join(host) + "\n\n#endif\n")
    extra = ["sqlite3_mprintf", "sqlite3_snprintf", "sqlite3_str_appendf", "sqlite3_config",
             "sqlite3_db_config", "sqlite3_initialize", "sqlite3_shutdown",
             "sqlite3_soft_heap_limit64", "sqlite3_temp_directory", "sqlite3_link_owner"]
    renh = (f"{head} For the host test: the\n"
            " * link library's functions under names of their own, so the program under\n"
            " * test calls them and they reach the real SQLite through the library's side.\n"
            " * MIT, Copyright (c) 2026 AmiCradle Team. */\n"
            + "".join(f"#define {n} lnk_{n}\n" for n in rename + extra))
    linkc = (f"{head} The SQLite C API through\n"
             " * sqlite.library: a program links this (libsqlite3_link.a) where it linked\n"
             " * libsqlite3.a; sqlite3_link_base.c opens the library for it and has the calls\n"
             " * with a variable number of arguments.\n"
             " * MIT, Copyright (c) 2026 AmiCradle Team. */\n"
             '#include "sqlite3_link_priv.h"\n\n'
             + "\n\n".join(link) + "\n")
    return {"library/sqlite_lib.sfd": "\n".join(sfd) + "\n==end\n",
            "library/sqlite_lib_calls.h": libh,
            "include/inline/sqlite.h": inl,
            "tests/host_inline.h": hosth,
            "tests/host_rename.h": renh,
            "library/link/sqlite3_link.c": linkc}, len(cs), last


def main():
    check = "--check" in sys.argv
    out, n, last = generate()
    stale = []
    for rel, text in out.items():
        path = ROOT / rel
        if not path.exists() or path.read_text() != text:
            stale.append(rel)
            if not check:
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(text)
    if check:
        if stale:
            sys.exit("gen-library: out of date: " + ", ".join(stale))
        print(f"GEN-LIBRARY PASS {n} calls, last bias 0x{last:x}")
    else:
        print(f"gen-library: {n} calls (0x{FIRST_BIAS:x} to 0x{last:x}); wrote {len(stale)} files")


if __name__ == "__main__":
    main()
