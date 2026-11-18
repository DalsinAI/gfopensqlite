/* Made by tools/gen-library.py from its table: don't edit. sqlite.library's calls,
 * each the C call with its arguments from their registers. A call that can
 * run SQL goes through sql_run (a stack of its own when the caller's is small).
 * When that stack can't be had the call answers its failure value; a call
 * with no answer to give (SQL_Detach) runs on the caller's stack instead.
 * MIT, Copyright (c) 2026 AmiCradle Team. */

SQL_ENTRY void * L_SQL_Attach(void)
{
    return sql_attach();
}

struct x_SQL_Detach { void *owner; };
static LONG d_SQL_Detach(APTR v)
{
    struct x_SQL_Detach *x = (struct x_SQL_Detach *)v;
    sql_detach(x->owner);
    return 0;
}
SQL_ENTRY void L_SQL_Detach(REG(a0, void *owner))
{
    struct x_SQL_Detach x;
    x.owner = owner;
    if (sql_run(d_SQL_Detach, &x)) d_SQL_Detach(&x);
}

SQL_ENTRY int L_SQL_DbConfig(REG(a0, sqlite3 *db), REG(d0, int op), REG(a1, void *p), REG(d1, int a), REG(d2, int b))
{
    return sql_db_config(db, op, p, a, b);
}

SQL_ENTRY const char * L_SQL_Libversion(void)
{
    return sqlite3_libversion();
}

SQL_ENTRY const char * L_SQL_Sourceid(void)
{
    return sqlite3_sourceid();
}

SQL_ENTRY int L_SQL_LibversionNumber(void)
{
    return sqlite3_libversion_number();
}

SQL_ENTRY int L_SQL_Threadsafe(void)
{
    return sqlite3_threadsafe();
}

struct x_SQL_Open { const char *filename; sqlite3 **ppDb; APTR owner; int r; };
static LONG d_SQL_Open(APTR v)
{
    struct x_SQL_Open *x = (struct x_SQL_Open *)v;
    x->r = sql_open(x->filename, x->ppDb, x->owner);
    return 0;
}
SQL_ENTRY int L_SQL_Open(REG(a0, const char *filename), REG(a1, sqlite3 **ppDb), REG(a2, APTR owner))
{
    struct x_SQL_Open x;
    x.filename = filename; x.ppDb = ppDb; x.owner = owner; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_Open, &x);
    return x.r;
}

struct x_SQL_OpenV2 { const char *filename; sqlite3 **ppDb; int flags; const char *zVfs; APTR owner; int r; };
static LONG d_SQL_OpenV2(APTR v)
{
    struct x_SQL_OpenV2 *x = (struct x_SQL_OpenV2 *)v;
    x->r = sql_open_v2(x->filename, x->ppDb, x->flags, x->zVfs, x->owner);
    return 0;
}
SQL_ENTRY int L_SQL_OpenV2(REG(a0, const char *filename), REG(a1, sqlite3 **ppDb), REG(d0, int flags), REG(a2, const char *zVfs), REG(a3, APTR owner))
{
    struct x_SQL_OpenV2 x;
    x.filename = filename; x.ppDb = ppDb; x.flags = flags; x.zVfs = zVfs; x.owner = owner; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_OpenV2, &x);
    return x.r;
}

struct x_SQL_Close { sqlite3 *db; int r; };
static LONG d_SQL_Close(APTR v)
{
    struct x_SQL_Close *x = (struct x_SQL_Close *)v;
    x->r = sql_close(x->db);
    return 0;
}
SQL_ENTRY int L_SQL_Close(REG(a0, sqlite3 *db))
{
    struct x_SQL_Close x;
    x.db = db; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_Close, &x);
    return x.r;
}

struct x_SQL_CloseV2 { sqlite3 *db; int r; };
static LONG d_SQL_CloseV2(APTR v)
{
    struct x_SQL_CloseV2 *x = (struct x_SQL_CloseV2 *)v;
    x->r = sql_close_v2(x->db);
    return 0;
}
SQL_ENTRY int L_SQL_CloseV2(REG(a0, sqlite3 *db))
{
    struct x_SQL_CloseV2 x;
    x.db = db; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_CloseV2, &x);
    return x.r;
}

struct x_SQL_Exec { sqlite3 *db; const char *sql; APTR callback; void *arg; char **errmsg; int r; };
static LONG d_SQL_Exec(APTR v)
{
    struct x_SQL_Exec *x = (struct x_SQL_Exec *)v;
    x->r = sqlite3_exec(x->db, x->sql, (int (*)(void*,int,char**,char**))x->callback, x->arg, x->errmsg);
    return 0;
}
SQL_ENTRY int L_SQL_Exec(REG(a0, sqlite3 *db), REG(a1, const char *sql), REG(a2, APTR callback), REG(a3, void *arg), REG(d0, char **errmsg))
{
    struct x_SQL_Exec x;
    x.db = db; x.sql = sql; x.callback = callback; x.arg = arg; x.errmsg = errmsg; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_Exec, &x);
    return x.r;
}

SQL_ENTRY int L_SQL_ExtendedResultCodes(REG(a0, sqlite3 *db), REG(d0, int onoff))
{
    return sqlite3_extended_result_codes(db, onoff);
}

SQL_ENTRY void L_SQL_LastInsertRowid(REG(a0, sqlite3 *db), REG(a1, sqlite3_int64 *lib_result))
{
    *lib_result = sqlite3_last_insert_rowid(db);
}

SQL_ENTRY void L_SQL_SetLastInsertRowid(REG(a0, sqlite3 *db), REG(d0, ULONG rowid_hi), REG(d1, ULONG rowid_lo))
{
    sqlite3_set_last_insert_rowid(db, (sqlite3_int64)(((sqlite3_uint64)rowid_hi << 32) | rowid_lo));
}

SQL_ENTRY int L_SQL_Changes(REG(a0, sqlite3 *db))
{
    return sqlite3_changes(db);
}

SQL_ENTRY void L_SQL_Changes64(REG(a0, sqlite3 *db), REG(a1, sqlite3_int64 *lib_result))
{
    *lib_result = sqlite3_changes64(db);
}

SQL_ENTRY int L_SQL_TotalChanges(REG(a0, sqlite3 *db))
{
    return sqlite3_total_changes(db);
}

SQL_ENTRY void L_SQL_TotalChanges64(REG(a0, sqlite3 *db), REG(a1, sqlite3_int64 *lib_result))
{
    *lib_result = sqlite3_total_changes64(db);
}

SQL_ENTRY void L_SQL_Interrupt(REG(a0, sqlite3 *db))
{
    sqlite3_interrupt(db);
}

SQL_ENTRY int L_SQL_IsInterrupted(REG(a0, sqlite3 *db))
{
    return sqlite3_is_interrupted(db);
}

SQL_ENTRY int L_SQL_Complete(REG(a0, const char *sql))
{
    return sqlite3_complete(sql);
}

SQL_ENTRY int L_SQL_BusyHandler(REG(a0, sqlite3 *db), REG(a1, APTR xBusy), REG(a2, void *arg))
{
    return sqlite3_busy_handler(db, (int (*)(void*,int))xBusy, arg);
}

SQL_ENTRY int L_SQL_BusyTimeout(REG(a0, sqlite3 *db), REG(d0, int ms))
{
    return sqlite3_busy_timeout(db, ms);
}

SQL_ENTRY void * L_SQL_Malloc(REG(d0, int n))
{
    return sqlite3_malloc(n);
}

SQL_ENTRY void * L_SQL_Malloc64(REG(d0, ULONG n_hi), REG(d1, ULONG n_lo))
{
    return sqlite3_malloc64((sqlite3_uint64)(((sqlite3_uint64)n_hi << 32) | n_lo));
}

SQL_ENTRY void * L_SQL_Realloc(REG(a0, void *p), REG(d0, int n))
{
    return sqlite3_realloc(p, n);
}

SQL_ENTRY void * L_SQL_Realloc64(REG(a0, void *p), REG(d0, ULONG n_hi), REG(d1, ULONG n_lo))
{
    return sqlite3_realloc64(p, (sqlite3_uint64)(((sqlite3_uint64)n_hi << 32) | n_lo));
}

SQL_ENTRY void L_SQL_Free(REG(a0, void *p))
{
    sqlite3_free(p);
}

SQL_ENTRY void L_SQL_Msize(REG(a0, void *p), REG(a1, sqlite3_uint64 *lib_result))
{
    *lib_result = sqlite3_msize(p);
}

SQL_ENTRY void L_SQL_MemoryUsed(REG(a0, sqlite3_int64 *lib_result))
{
    *lib_result = sqlite3_memory_used();
}

SQL_ENTRY void L_SQL_MemoryHighwater(REG(d0, int resetFlag), REG(a0, sqlite3_int64 *lib_result))
{
    *lib_result = sqlite3_memory_highwater(resetFlag);
}

SQL_ENTRY void L_SQL_Randomness(REG(d0, int N), REG(a0, void *P))
{
    sqlite3_randomness(N, P);
}

SQL_ENTRY int L_SQL_SetAuthorizer(REG(a0, sqlite3 *db), REG(a1, APTR xAuth), REG(a2, void *pUserData))
{
    return sqlite3_set_authorizer(db, (int (*)(void*,int,const char*,const char*,const char*,const char*))xAuth, pUserData);
}

SQL_ENTRY void L_SQL_ProgressHandler(REG(a0, sqlite3 *db), REG(d0, int nOps), REG(a1, APTR xProgress), REG(a2, void *pArg))
{
    sqlite3_progress_handler(db, nOps, (int (*)(void*))xProgress, pArg);
}

SQL_ENTRY int L_SQL_Errcode(REG(a0, sqlite3 *db))
{
    return sqlite3_errcode(db);
}

SQL_ENTRY int L_SQL_ExtendedErrcode(REG(a0, sqlite3 *db))
{
    return sqlite3_extended_errcode(db);
}

SQL_ENTRY const char * L_SQL_Errmsg(REG(a0, sqlite3 *db))
{
    return sqlite3_errmsg(db);
}

SQL_ENTRY const char * L_SQL_Errstr(REG(d0, int rc))
{
    return sqlite3_errstr(rc);
}

SQL_ENTRY int L_SQL_ErrorOffset(REG(a0, sqlite3 *db))
{
    return sqlite3_error_offset(db);
}

SQL_ENTRY int L_SQL_SystemErrno(REG(a0, sqlite3 *db))
{
    return sqlite3_system_errno(db);
}

SQL_ENTRY int L_SQL_Limit(REG(a0, sqlite3 *db), REG(d0, int id), REG(d1, int newVal))
{
    return sqlite3_limit(db, id, newVal);
}

struct x_SQL_PrepareV2 { sqlite3 *db; const char *zSql; int nByte; sqlite3_stmt **ppStmt; const char **pzTail; int r; };
static LONG d_SQL_PrepareV2(APTR v)
{
    struct x_SQL_PrepareV2 *x = (struct x_SQL_PrepareV2 *)v;
    x->r = sqlite3_prepare_v2(x->db, x->zSql, x->nByte, x->ppStmt, x->pzTail);
    return 0;
}
SQL_ENTRY int L_SQL_PrepareV2(REG(a0, sqlite3 *db), REG(a1, const char *zSql), REG(d0, int nByte), REG(a2, sqlite3_stmt **ppStmt), REG(a3, const char **pzTail))
{
    struct x_SQL_PrepareV2 x;
    x.db = db; x.zSql = zSql; x.nByte = nByte; x.ppStmt = ppStmt; x.pzTail = pzTail; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_PrepareV2, &x);
    return x.r;
}

struct x_SQL_PrepareV3 { sqlite3 *db; const char *zSql; int nByte; unsigned int prepFlags; sqlite3_stmt **ppStmt; const char **pzTail; int r; };
static LONG d_SQL_PrepareV3(APTR v)
{
    struct x_SQL_PrepareV3 *x = (struct x_SQL_PrepareV3 *)v;
    x->r = sqlite3_prepare_v3(x->db, x->zSql, x->nByte, x->prepFlags, x->ppStmt, x->pzTail);
    return 0;
}
SQL_ENTRY int L_SQL_PrepareV3(REG(a0, sqlite3 *db), REG(a1, const char *zSql), REG(d0, int nByte), REG(d1, unsigned int prepFlags), REG(a2, sqlite3_stmt **ppStmt), REG(a3, const char **pzTail))
{
    struct x_SQL_PrepareV3 x;
    x.db = db; x.zSql = zSql; x.nByte = nByte; x.prepFlags = prepFlags; x.ppStmt = ppStmt; x.pzTail = pzTail; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_PrepareV3, &x);
    return x.r;
}

SQL_ENTRY const char * L_SQL_Sql(REG(a0, sqlite3_stmt *pStmt))
{
    return sqlite3_sql(pStmt);
}

SQL_ENTRY char * L_SQL_ExpandedSql(REG(a0, sqlite3_stmt *pStmt))
{
    return sqlite3_expanded_sql(pStmt);
}

SQL_ENTRY int L_SQL_StmtReadonly(REG(a0, sqlite3_stmt *pStmt))
{
    return sqlite3_stmt_readonly(pStmt);
}

SQL_ENTRY int L_SQL_StmtIsexplain(REG(a0, sqlite3_stmt *pStmt))
{
    return sqlite3_stmt_isexplain(pStmt);
}

SQL_ENTRY int L_SQL_StmtBusy(REG(a0, sqlite3_stmt *pStmt))
{
    return sqlite3_stmt_busy(pStmt);
}

SQL_ENTRY int L_SQL_StmtStatus(REG(a0, sqlite3_stmt *pStmt), REG(d0, int op), REG(d1, int resetFlg))
{
    return sqlite3_stmt_status(pStmt, op, resetFlg);
}

SQL_ENTRY int L_SQL_BindBlob(REG(a0, sqlite3_stmt *pStmt), REG(d0, int idx), REG(a1, const void *data), REG(d1, int n), REG(a2, APTR xDel))
{
    return sqlite3_bind_blob(pStmt, idx, data, n, (void (*)(void*))xDel);
}

SQL_ENTRY int L_SQL_BindBlob64(REG(a0, sqlite3_stmt *pStmt), REG(d0, int idx), REG(a1, const void *data), REG(d1, ULONG n_hi), REG(d2, ULONG n_lo), REG(a2, APTR xDel))
{
    return sqlite3_bind_blob64(pStmt, idx, data, (sqlite3_uint64)(((sqlite3_uint64)n_hi << 32) | n_lo), (void (*)(void*))xDel);
}

SQL_ENTRY int L_SQL_BindDouble(REG(a0, sqlite3_stmt *pStmt), REG(d0, int idx), REG(a1, const double *value_p))
{
    return sqlite3_bind_double(pStmt, idx, *value_p);
}

SQL_ENTRY int L_SQL_BindInt(REG(a0, sqlite3_stmt *pStmt), REG(d0, int idx), REG(d1, int value))
{
    return sqlite3_bind_int(pStmt, idx, value);
}

SQL_ENTRY int L_SQL_BindInt64(REG(a0, sqlite3_stmt *pStmt), REG(d0, int idx), REG(d1, ULONG value_hi), REG(d2, ULONG value_lo))
{
    return sqlite3_bind_int64(pStmt, idx, (sqlite3_int64)(((sqlite3_uint64)value_hi << 32) | value_lo));
}

SQL_ENTRY int L_SQL_BindNull(REG(a0, sqlite3_stmt *pStmt), REG(d0, int idx))
{
    return sqlite3_bind_null(pStmt, idx);
}

SQL_ENTRY int L_SQL_BindText(REG(a0, sqlite3_stmt *pStmt), REG(d0, int idx), REG(a1, const char *data), REG(d1, int n), REG(a2, APTR xDel))
{
    return sqlite3_bind_text(pStmt, idx, data, n, (void (*)(void*))xDel);
}

SQL_ENTRY int L_SQL_BindText64(REG(a0, sqlite3_stmt *pStmt), REG(d0, int idx), REG(a1, const char *data), REG(d1, ULONG n_hi), REG(d2, ULONG n_lo), REG(a2, APTR xDel), REG(d3, unsigned char encoding))
{
    return sqlite3_bind_text64(pStmt, idx, data, (sqlite3_uint64)(((sqlite3_uint64)n_hi << 32) | n_lo), (void (*)(void*))xDel, encoding);
}

SQL_ENTRY int L_SQL_BindValue(REG(a0, sqlite3_stmt *pStmt), REG(d0, int idx), REG(a1, const sqlite3_value *value))
{
    return sqlite3_bind_value(pStmt, idx, value);
}

SQL_ENTRY int L_SQL_BindZeroblob(REG(a0, sqlite3_stmt *pStmt), REG(d0, int idx), REG(d1, int n))
{
    return sqlite3_bind_zeroblob(pStmt, idx, n);
}

SQL_ENTRY int L_SQL_BindParameterCount(REG(a0, sqlite3_stmt *pStmt))
{
    return sqlite3_bind_parameter_count(pStmt);
}

SQL_ENTRY const char * L_SQL_BindParameterName(REG(a0, sqlite3_stmt *pStmt), REG(d0, int idx))
{
    return sqlite3_bind_parameter_name(pStmt, idx);
}

SQL_ENTRY int L_SQL_BindParameterIndex(REG(a0, sqlite3_stmt *pStmt), REG(a1, const char *zName))
{
    return sqlite3_bind_parameter_index(pStmt, zName);
}

SQL_ENTRY int L_SQL_ClearBindings(REG(a0, sqlite3_stmt *pStmt))
{
    return sqlite3_clear_bindings(pStmt);
}

SQL_ENTRY int L_SQL_ColumnCount(REG(a0, sqlite3_stmt *pStmt))
{
    return sqlite3_column_count(pStmt);
}

SQL_ENTRY const char * L_SQL_ColumnName(REG(a0, sqlite3_stmt *pStmt), REG(d0, int N))
{
    return sqlite3_column_name(pStmt, N);
}

SQL_ENTRY const char * L_SQL_ColumnDecltype(REG(a0, sqlite3_stmt *pStmt), REG(d0, int N))
{
    return sqlite3_column_decltype(pStmt, N);
}

struct x_SQL_Step { sqlite3_stmt *pStmt; int r; };
static LONG d_SQL_Step(APTR v)
{
    struct x_SQL_Step *x = (struct x_SQL_Step *)v;
    x->r = sqlite3_step(x->pStmt);
    return 0;
}
SQL_ENTRY int L_SQL_Step(REG(a0, sqlite3_stmt *pStmt))
{
    struct x_SQL_Step x;
    x.pStmt = pStmt; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_Step, &x);
    return x.r;
}

SQL_ENTRY int L_SQL_DataCount(REG(a0, sqlite3_stmt *pStmt))
{
    return sqlite3_data_count(pStmt);
}

SQL_ENTRY const void * L_SQL_ColumnBlob(REG(a0, sqlite3_stmt *pStmt), REG(d0, int iCol))
{
    return sqlite3_column_blob(pStmt, iCol);
}

SQL_ENTRY void L_SQL_ColumnDouble(REG(a0, sqlite3_stmt *pStmt), REG(d0, int iCol), REG(a1, double *lib_result))
{
    *lib_result = sqlite3_column_double(pStmt, iCol);
}

SQL_ENTRY int L_SQL_ColumnInt(REG(a0, sqlite3_stmt *pStmt), REG(d0, int iCol))
{
    return sqlite3_column_int(pStmt, iCol);
}

SQL_ENTRY void L_SQL_ColumnInt64(REG(a0, sqlite3_stmt *pStmt), REG(d0, int iCol), REG(a1, sqlite3_int64 *lib_result))
{
    *lib_result = sqlite3_column_int64(pStmt, iCol);
}

SQL_ENTRY const unsigned char * L_SQL_ColumnText(REG(a0, sqlite3_stmt *pStmt), REG(d0, int iCol))
{
    return sqlite3_column_text(pStmt, iCol);
}

SQL_ENTRY sqlite3_value * L_SQL_ColumnValue(REG(a0, sqlite3_stmt *pStmt), REG(d0, int iCol))
{
    return sqlite3_column_value(pStmt, iCol);
}

SQL_ENTRY int L_SQL_ColumnBytes(REG(a0, sqlite3_stmt *pStmt), REG(d0, int iCol))
{
    return sqlite3_column_bytes(pStmt, iCol);
}

SQL_ENTRY int L_SQL_ColumnType(REG(a0, sqlite3_stmt *pStmt), REG(d0, int iCol))
{
    return sqlite3_column_type(pStmt, iCol);
}

struct x_SQL_Finalize { sqlite3_stmt *pStmt; int r; };
static LONG d_SQL_Finalize(APTR v)
{
    struct x_SQL_Finalize *x = (struct x_SQL_Finalize *)v;
    x->r = sqlite3_finalize(x->pStmt);
    return 0;
}
SQL_ENTRY int L_SQL_Finalize(REG(a0, sqlite3_stmt *pStmt))
{
    struct x_SQL_Finalize x;
    x.pStmt = pStmt; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_Finalize, &x);
    return x.r;
}

struct x_SQL_Reset { sqlite3_stmt *pStmt; int r; };
static LONG d_SQL_Reset(APTR v)
{
    struct x_SQL_Reset *x = (struct x_SQL_Reset *)v;
    x->r = sqlite3_reset(x->pStmt);
    return 0;
}
SQL_ENTRY int L_SQL_Reset(REG(a0, sqlite3_stmt *pStmt))
{
    struct x_SQL_Reset x;
    x.pStmt = pStmt; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_Reset, &x);
    return x.r;
}

SQL_ENTRY int L_SQL_CreateFunction(REG(a0, sqlite3 *db), REG(a1, const char *zFunctionName), REG(d0, int nArg), REG(d1, int eTextRep), REG(a2, void *pApp), REG(a3, APTR xFunc), REG(d2, APTR xStep), REG(d3, APTR xFinal))
{
    return sqlite3_create_function(db, zFunctionName, nArg, eTextRep, pApp, (void (*)(sqlite3_context*,int,sqlite3_value**))xFunc, (void (*)(sqlite3_context*,int,sqlite3_value**))xStep, (void (*)(sqlite3_context*))xFinal);
}

SQL_ENTRY int L_SQL_CreateFunctionV2(REG(a0, sqlite3 *db), REG(a1, const char *zFunctionName), REG(d0, int nArg), REG(d1, int eTextRep), REG(a2, void *pApp), REG(a3, APTR xFunc), REG(d2, APTR xStep), REG(d3, APTR xFinal), REG(d4, APTR xDestroy))
{
    return sqlite3_create_function_v2(db, zFunctionName, nArg, eTextRep, pApp, (void (*)(sqlite3_context*,int,sqlite3_value**))xFunc, (void (*)(sqlite3_context*,int,sqlite3_value**))xStep, (void (*)(sqlite3_context*))xFinal, (void (*)(void*))xDestroy);
}

SQL_ENTRY const void * L_SQL_ValueBlob(REG(a0, sqlite3_value *pVal))
{
    return sqlite3_value_blob(pVal);
}

SQL_ENTRY void L_SQL_ValueDouble(REG(a0, sqlite3_value *pVal), REG(a1, double *lib_result))
{
    *lib_result = sqlite3_value_double(pVal);
}

SQL_ENTRY int L_SQL_ValueInt(REG(a0, sqlite3_value *pVal))
{
    return sqlite3_value_int(pVal);
}

SQL_ENTRY void L_SQL_ValueInt64(REG(a0, sqlite3_value *pVal), REG(a1, sqlite3_int64 *lib_result))
{
    *lib_result = sqlite3_value_int64(pVal);
}

SQL_ENTRY const unsigned char * L_SQL_ValueText(REG(a0, sqlite3_value *pVal))
{
    return sqlite3_value_text(pVal);
}

SQL_ENTRY int L_SQL_ValueBytes(REG(a0, sqlite3_value *pVal))
{
    return sqlite3_value_bytes(pVal);
}

SQL_ENTRY int L_SQL_ValueType(REG(a0, sqlite3_value *pVal))
{
    return sqlite3_value_type(pVal);
}

SQL_ENTRY int L_SQL_ValueNumericType(REG(a0, sqlite3_value *pVal))
{
    return sqlite3_value_numeric_type(pVal);
}

SQL_ENTRY sqlite3_value * L_SQL_ValueDup(REG(a0, const sqlite3_value *pVal))
{
    return sqlite3_value_dup(pVal);
}

SQL_ENTRY void L_SQL_ValueFree(REG(a0, sqlite3_value *pVal))
{
    sqlite3_value_free(pVal);
}

SQL_ENTRY void * L_SQL_AggregateContext(REG(a0, sqlite3_context *ctx), REG(d0, int nBytes))
{
    return sqlite3_aggregate_context(ctx, nBytes);
}

SQL_ENTRY void * L_SQL_UserData(REG(a0, sqlite3_context *ctx))
{
    return sqlite3_user_data(ctx);
}

SQL_ENTRY sqlite3 * L_SQL_ContextDbHandle(REG(a0, sqlite3_context *ctx))
{
    return sqlite3_context_db_handle(ctx);
}

SQL_ENTRY void L_SQL_ResultBlob(REG(a0, sqlite3_context *ctx), REG(a1, const void *data), REG(d0, int n), REG(a2, APTR xDel))
{
    sqlite3_result_blob(ctx, data, n, (void (*)(void*))xDel);
}

SQL_ENTRY void L_SQL_ResultDouble(REG(a0, sqlite3_context *ctx), REG(a1, const double *value_p))
{
    sqlite3_result_double(ctx, *value_p);
}

SQL_ENTRY void L_SQL_ResultError(REG(a0, sqlite3_context *ctx), REG(a1, const char *msg), REG(d0, int n))
{
    sqlite3_result_error(ctx, msg, n);
}

SQL_ENTRY void L_SQL_ResultErrorToobig(REG(a0, sqlite3_context *ctx))
{
    sqlite3_result_error_toobig(ctx);
}

SQL_ENTRY void L_SQL_ResultErrorNomem(REG(a0, sqlite3_context *ctx))
{
    sqlite3_result_error_nomem(ctx);
}

SQL_ENTRY void L_SQL_ResultErrorCode(REG(a0, sqlite3_context *ctx), REG(d0, int code))
{
    sqlite3_result_error_code(ctx, code);
}

SQL_ENTRY void L_SQL_ResultInt(REG(a0, sqlite3_context *ctx), REG(d0, int value))
{
    sqlite3_result_int(ctx, value);
}

SQL_ENTRY void L_SQL_ResultInt64(REG(a0, sqlite3_context *ctx), REG(d0, ULONG value_hi), REG(d1, ULONG value_lo))
{
    sqlite3_result_int64(ctx, (sqlite3_int64)(((sqlite3_uint64)value_hi << 32) | value_lo));
}

SQL_ENTRY void L_SQL_ResultNull(REG(a0, sqlite3_context *ctx))
{
    sqlite3_result_null(ctx);
}

SQL_ENTRY void L_SQL_ResultText(REG(a0, sqlite3_context *ctx), REG(a1, const char *data), REG(d0, int n), REG(a2, APTR xDel))
{
    sqlite3_result_text(ctx, data, n, (void (*)(void*))xDel);
}

SQL_ENTRY void L_SQL_ResultValue(REG(a0, sqlite3_context *ctx), REG(a1, sqlite3_value *value))
{
    sqlite3_result_value(ctx, value);
}

SQL_ENTRY void L_SQL_ResultZeroblob(REG(a0, sqlite3_context *ctx), REG(d0, int n))
{
    sqlite3_result_zeroblob(ctx, n);
}

SQL_ENTRY int L_SQL_CreateCollation(REG(a0, sqlite3 *db), REG(a1, const char *zName), REG(d0, int eTextRep), REG(a2, void *pArg), REG(a3, APTR xCompare))
{
    return sqlite3_create_collation(db, zName, eTextRep, pArg, (int (*)(void*,int,const void*,int,const void*))xCompare);
}

SQL_ENTRY int L_SQL_CreateCollationV2(REG(a0, sqlite3 *db), REG(a1, const char *zName), REG(d0, int eTextRep), REG(a2, void *pArg), REG(a3, APTR xCompare), REG(d1, APTR xDestroy))
{
    return sqlite3_create_collation_v2(db, zName, eTextRep, pArg, (int (*)(void*,int,const void*,int,const void*))xCompare, (void (*)(void*))xDestroy);
}

SQL_ENTRY int L_SQL_GetAutocommit(REG(a0, sqlite3 *db))
{
    return sqlite3_get_autocommit(db);
}

SQL_ENTRY int L_SQL_TxnState(REG(a0, sqlite3 *db), REG(a1, const char *zSchema))
{
    return sqlite3_txn_state(db, zSchema);
}

SQL_ENTRY sqlite3 * L_SQL_DbHandle(REG(a0, sqlite3_stmt *pStmt))
{
    return sqlite3_db_handle(pStmt);
}

SQL_ENTRY const char * L_SQL_DbFilename(REG(a0, sqlite3 *db), REG(a1, const char *zDbName))
{
    return sqlite3_db_filename(db, zDbName);
}

SQL_ENTRY int L_SQL_DbReadonly(REG(a0, sqlite3 *db), REG(a1, const char *zDbName))
{
    return sqlite3_db_readonly(db, zDbName);
}

SQL_ENTRY sqlite3_stmt * L_SQL_NextStmt(REG(a0, sqlite3 *db), REG(a1, sqlite3_stmt *pStmt))
{
    return sqlite3_next_stmt(db, pStmt);
}

SQL_ENTRY void * L_SQL_CommitHook(REG(a0, sqlite3 *db), REG(a1, APTR xCallback), REG(a2, void *pArg))
{
    return sqlite3_commit_hook(db, (int (*)(void*))xCallback, pArg);
}

SQL_ENTRY void * L_SQL_RollbackHook(REG(a0, sqlite3 *db), REG(a1, APTR xCallback), REG(a2, void *pArg))
{
    return sqlite3_rollback_hook(db, (void (*)(void*))xCallback, pArg);
}

SQL_ENTRY void * L_SQL_UpdateHook(REG(a0, sqlite3 *db), REG(a1, APTR xCallback), REG(a2, void *pArg))
{
    return sqlite3_update_hook(db, (void (*)(void*,int,const char*,const char*,sqlite3_int64))xCallback, pArg);
}

SQL_ENTRY int L_SQL_TraceV2(REG(a0, sqlite3 *db), REG(d0, unsigned int uMask), REG(a1, APTR xCallback), REG(a2, void *pCtx))
{
    return sqlite3_trace_v2(db, uMask, (int (*)(unsigned int,void*,void*,void*))xCallback, pCtx);
}

struct x_SQL_DbReleaseMemory { sqlite3 *db; int r; };
static LONG d_SQL_DbReleaseMemory(APTR v)
{
    struct x_SQL_DbReleaseMemory *x = (struct x_SQL_DbReleaseMemory *)v;
    x->r = sqlite3_db_release_memory(x->db);
    return 0;
}
SQL_ENTRY int L_SQL_DbReleaseMemory(REG(a0, sqlite3 *db))
{
    struct x_SQL_DbReleaseMemory x;
    x.db = db; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_DbReleaseMemory, &x);
    return x.r;
}

struct x_SQL_DbCacheflush { sqlite3 *db; int r; };
static LONG d_SQL_DbCacheflush(APTR v)
{
    struct x_SQL_DbCacheflush *x = (struct x_SQL_DbCacheflush *)v;
    x->r = sqlite3_db_cacheflush(x->db);
    return 0;
}
SQL_ENTRY int L_SQL_DbCacheflush(REG(a0, sqlite3 *db))
{
    struct x_SQL_DbCacheflush x;
    x.db = db; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_DbCacheflush, &x);
    return x.r;
}

struct x_SQL_FileControl { sqlite3 *db; const char *zDbName; int op; void *pArg; int r; };
static LONG d_SQL_FileControl(APTR v)
{
    struct x_SQL_FileControl *x = (struct x_SQL_FileControl *)v;
    x->r = sqlite3_file_control(x->db, x->zDbName, x->op, x->pArg);
    return 0;
}
SQL_ENTRY int L_SQL_FileControl(REG(a0, sqlite3 *db), REG(a1, const char *zDbName), REG(d0, int op), REG(a2, void *pArg))
{
    struct x_SQL_FileControl x;
    x.db = db; x.zDbName = zDbName; x.op = op; x.pArg = pArg; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_FileControl, &x);
    return x.r;
}

SQL_ENTRY void * L_SQL_WalHook(REG(a0, sqlite3 *db), REG(a1, APTR xCallback), REG(a2, void *pArg))
{
    return sqlite3_wal_hook(db, (int (*)(void*,sqlite3*,const char*,int))xCallback, pArg);
}

struct x_SQL_WalCheckpointV2 { sqlite3 *db; const char *zDb; int eMode; int *pnLog; int *pnCkpt; int r; };
static LONG d_SQL_WalCheckpointV2(APTR v)
{
    struct x_SQL_WalCheckpointV2 *x = (struct x_SQL_WalCheckpointV2 *)v;
    x->r = sqlite3_wal_checkpoint_v2(x->db, x->zDb, x->eMode, x->pnLog, x->pnCkpt);
    return 0;
}
SQL_ENTRY int L_SQL_WalCheckpointV2(REG(a0, sqlite3 *db), REG(a1, const char *zDb), REG(d0, int eMode), REG(a2, int *pnLog), REG(a3, int *pnCkpt))
{
    struct x_SQL_WalCheckpointV2 x;
    x.db = db; x.zDb = zDb; x.eMode = eMode; x.pnLog = pnLog; x.pnCkpt = pnCkpt; x.r = SQLITE_NOMEM;
    sql_run(d_SQL_WalCheckpointV2, &x);
    return x.r;
}

SQL_ENTRY sqlite3_str * L_SQL_StrNew(REG(a0, sqlite3 *db))
{
    return sqlite3_str_new(db);
}

SQL_ENTRY char * L_SQL_StrFinish(REG(a0, sqlite3_str *p))
{
    return sqlite3_str_finish(p);
}

SQL_ENTRY void L_SQL_StrVappendf(REG(a0, sqlite3_str *p), REG(a1, const char *zFormat), REG(a2, va_list ap))
{
    sqlite3_str_vappendf(p, zFormat, ap);
}

SQL_ENTRY void L_SQL_StrAppend(REG(a0, sqlite3_str *p), REG(a1, const char *zIn), REG(d0, int N))
{
    sqlite3_str_append(p, zIn, N);
}

SQL_ENTRY void L_SQL_StrAppendall(REG(a0, sqlite3_str *p), REG(a1, const char *zIn))
{
    sqlite3_str_appendall(p, zIn);
}

SQL_ENTRY void L_SQL_StrAppendchar(REG(a0, sqlite3_str *p), REG(d0, int N), REG(d1, char C))
{
    sqlite3_str_appendchar(p, N, C);
}

SQL_ENTRY void L_SQL_StrReset(REG(a0, sqlite3_str *p))
{
    sqlite3_str_reset(p);
}

SQL_ENTRY int L_SQL_StrErrcode(REG(a0, sqlite3_str *p))
{
    return sqlite3_str_errcode(p);
}

SQL_ENTRY int L_SQL_StrLength(REG(a0, sqlite3_str *p))
{
    return sqlite3_str_length(p);
}

SQL_ENTRY char * L_SQL_StrValue(REG(a0, sqlite3_str *p))
{
    return sqlite3_str_value(p);
}

SQL_ENTRY char * L_SQL_Vmprintf(REG(a0, const char *zFormat), REG(a1, va_list ap))
{
    return sqlite3_vmprintf(zFormat, ap);
}

SQL_ENTRY char * L_SQL_Vsnprintf(REG(d0, int n), REG(a0, char *zBuf), REG(a1, const char *zFormat), REG(a2, va_list ap))
{
    return sqlite3_vsnprintf(n, zBuf, zFormat, ap);
}

SQL_ENTRY int L_SQL_Stricmp(REG(a0, const char *a), REG(a1, const char *b))
{
    return sqlite3_stricmp(a, b);
}

SQL_ENTRY int L_SQL_Strnicmp(REG(a0, const char *a), REG(a1, const char *b), REG(d0, int n))
{
    return sqlite3_strnicmp(a, b, n);
}

SQL_ENTRY int L_SQL_Strglob(REG(a0, const char *zGlob), REG(a1, const char *zStr))
{
    return sqlite3_strglob(zGlob, zStr);
}

SQL_ENTRY int L_SQL_Strlike(REG(a0, const char *zGlob), REG(a1, const char *zStr), REG(d0, unsigned int cEsc))
{
    return sqlite3_strlike(zGlob, zStr, cEsc);
}

SQL_ENTRY int L_SQL_Sleep(REG(d0, int ms))
{
    return sqlite3_sleep(ms);
}

SQL_ENTRY sqlite3_vfs * L_SQL_VfsFind(REG(a0, const char *zVfsName))
{
    return sqlite3_vfs_find(zVfsName);
}

/* the last call's bias and the number of calls: checked against the table */
#define SQL_LAST_BIAS 0x366
#define SQL_CALLS 141

#define SQL_VECTORS \
    (APTR)L_SQL_Attach, (APTR)L_SQL_Detach, (APTR)L_SQL_DbConfig, \
    (APTR)L_SQL_Libversion, (APTR)L_SQL_Sourceid, (APTR)L_SQL_LibversionNumber, \
    (APTR)L_SQL_Threadsafe, (APTR)L_SQL_Open, (APTR)L_SQL_OpenV2, \
    (APTR)L_SQL_Close, (APTR)L_SQL_CloseV2, (APTR)L_SQL_Exec, \
    (APTR)L_SQL_ExtendedResultCodes, (APTR)L_SQL_LastInsertRowid, (APTR)L_SQL_SetLastInsertRowid, \
    (APTR)L_SQL_Changes, (APTR)L_SQL_Changes64, (APTR)L_SQL_TotalChanges, \
    (APTR)L_SQL_TotalChanges64, (APTR)L_SQL_Interrupt, (APTR)L_SQL_IsInterrupted, \
    (APTR)L_SQL_Complete, (APTR)L_SQL_BusyHandler, (APTR)L_SQL_BusyTimeout, \
    (APTR)L_SQL_Malloc, (APTR)L_SQL_Malloc64, (APTR)L_SQL_Realloc, \
    (APTR)L_SQL_Realloc64, (APTR)L_SQL_Free, (APTR)L_SQL_Msize, \
    (APTR)L_SQL_MemoryUsed, (APTR)L_SQL_MemoryHighwater, (APTR)L_SQL_Randomness, \
    (APTR)L_SQL_SetAuthorizer, (APTR)L_SQL_ProgressHandler, (APTR)L_SQL_Errcode, \
    (APTR)L_SQL_ExtendedErrcode, (APTR)L_SQL_Errmsg, (APTR)L_SQL_Errstr, \
    (APTR)L_SQL_ErrorOffset, (APTR)L_SQL_SystemErrno, (APTR)L_SQL_Limit, \
    (APTR)L_SQL_PrepareV2, (APTR)L_SQL_PrepareV3, (APTR)L_SQL_Sql, \
    (APTR)L_SQL_ExpandedSql, (APTR)L_SQL_StmtReadonly, (APTR)L_SQL_StmtIsexplain, \
    (APTR)L_SQL_StmtBusy, (APTR)L_SQL_StmtStatus, (APTR)L_SQL_BindBlob, \
    (APTR)L_SQL_BindBlob64, (APTR)L_SQL_BindDouble, (APTR)L_SQL_BindInt, \
    (APTR)L_SQL_BindInt64, (APTR)L_SQL_BindNull, (APTR)L_SQL_BindText, \
    (APTR)L_SQL_BindText64, (APTR)L_SQL_BindValue, (APTR)L_SQL_BindZeroblob, \
    (APTR)L_SQL_BindParameterCount, (APTR)L_SQL_BindParameterName, (APTR)L_SQL_BindParameterIndex, \
    (APTR)L_SQL_ClearBindings, (APTR)L_SQL_ColumnCount, (APTR)L_SQL_ColumnName, \
    (APTR)L_SQL_ColumnDecltype, (APTR)L_SQL_Step, (APTR)L_SQL_DataCount, \
    (APTR)L_SQL_ColumnBlob, (APTR)L_SQL_ColumnDouble, (APTR)L_SQL_ColumnInt, \
    (APTR)L_SQL_ColumnInt64, (APTR)L_SQL_ColumnText, (APTR)L_SQL_ColumnValue, \
    (APTR)L_SQL_ColumnBytes, (APTR)L_SQL_ColumnType, (APTR)L_SQL_Finalize, \
    (APTR)L_SQL_Reset, (APTR)L_SQL_CreateFunction, (APTR)L_SQL_CreateFunctionV2, \
    (APTR)L_SQL_ValueBlob, (APTR)L_SQL_ValueDouble, (APTR)L_SQL_ValueInt, \
    (APTR)L_SQL_ValueInt64, (APTR)L_SQL_ValueText, (APTR)L_SQL_ValueBytes, \
    (APTR)L_SQL_ValueType, (APTR)L_SQL_ValueNumericType, (APTR)L_SQL_ValueDup, \
    (APTR)L_SQL_ValueFree, (APTR)L_SQL_AggregateContext, (APTR)L_SQL_UserData, \
    (APTR)L_SQL_ContextDbHandle, (APTR)L_SQL_ResultBlob, (APTR)L_SQL_ResultDouble, \
    (APTR)L_SQL_ResultError, (APTR)L_SQL_ResultErrorToobig, (APTR)L_SQL_ResultErrorNomem, \
    (APTR)L_SQL_ResultErrorCode, (APTR)L_SQL_ResultInt, (APTR)L_SQL_ResultInt64, \
    (APTR)L_SQL_ResultNull, (APTR)L_SQL_ResultText, (APTR)L_SQL_ResultValue, \
    (APTR)L_SQL_ResultZeroblob, (APTR)L_SQL_CreateCollation, (APTR)L_SQL_CreateCollationV2, \
    (APTR)L_SQL_GetAutocommit, (APTR)L_SQL_TxnState, (APTR)L_SQL_DbHandle, \
    (APTR)L_SQL_DbFilename, (APTR)L_SQL_DbReadonly, (APTR)L_SQL_NextStmt, \
    (APTR)L_SQL_CommitHook, (APTR)L_SQL_RollbackHook, (APTR)L_SQL_UpdateHook, \
    (APTR)L_SQL_TraceV2, (APTR)L_SQL_DbReleaseMemory, (APTR)L_SQL_DbCacheflush, \
    (APTR)L_SQL_FileControl, (APTR)L_SQL_WalHook, (APTR)L_SQL_WalCheckpointV2, \
    (APTR)L_SQL_StrNew, (APTR)L_SQL_StrFinish, (APTR)L_SQL_StrVappendf, \
    (APTR)L_SQL_StrAppend, (APTR)L_SQL_StrAppendall, (APTR)L_SQL_StrAppendchar, \
    (APTR)L_SQL_StrReset, (APTR)L_SQL_StrErrcode, (APTR)L_SQL_StrLength, \
    (APTR)L_SQL_StrValue, (APTR)L_SQL_Vmprintf, (APTR)L_SQL_Vsnprintf, \
    (APTR)L_SQL_Stricmp, (APTR)L_SQL_Strnicmp, (APTR)L_SQL_Strglob, \
    (APTR)L_SQL_Strlike, (APTR)L_SQL_Sleep, (APTR)L_SQL_VfsFind
