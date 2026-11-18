/* Made by tools/gen-library.py from its table: don't edit. The SQLite C API through
 * sqlite.library: a program links this (libsqlite3_link.a) where it linked
 * libsqlite3.a; sqlite3_link_base.c opens the library for it and has the calls
 * with a variable number of arguments.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "sqlite3_link_priv.h"

const char *sqlite3_libversion(void)
{
    return SQL_Libversion();
}

const char *sqlite3_sourceid(void)
{
    return SQL_Sourceid();
}

int sqlite3_libversion_number(void)
{
    return SQL_LibversionNumber();
}

int sqlite3_threadsafe(void)
{
    return SQL_Threadsafe();
}

int sqlite3_open(const char *filename, sqlite3 **ppDb)
{
    return SQL_Open(filename, ppDb, sqlite3_link_owner);
}

int sqlite3_open_v2(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs)
{
    return SQL_OpenV2(filename, ppDb, flags, zVfs, sqlite3_link_owner);
}

int sqlite3_close(sqlite3 *db)
{
    return SQL_Close(db);
}

int sqlite3_close_v2(sqlite3 *db)
{
    return SQL_CloseV2(db);
}

int sqlite3_exec(sqlite3 *db, const char *sql, int (*callback)(void*,int,char**,char**), void *arg, char **errmsg)
{
    return SQL_Exec(db, sql, (APTR)callback, arg, errmsg);
}

int sqlite3_extended_result_codes(sqlite3 *db, int onoff)
{
    return SQL_ExtendedResultCodes(db, onoff);
}

sqlite3_int64 sqlite3_last_insert_rowid(sqlite3 *db)
{
    sqlite3_int64 lib_result = 0;
    SQL_LastInsertRowid(db, &lib_result);
    return lib_result;
}

void sqlite3_set_last_insert_rowid(sqlite3 *db, sqlite3_int64 rowid)
{
    SQL_SetLastInsertRowid(db, (ULONG)((sqlite3_uint64)rowid >> 32), (ULONG)rowid);
}

int sqlite3_changes(sqlite3 *db)
{
    return SQL_Changes(db);
}

sqlite3_int64 sqlite3_changes64(sqlite3 *db)
{
    sqlite3_int64 lib_result = 0;
    SQL_Changes64(db, &lib_result);
    return lib_result;
}

int sqlite3_total_changes(sqlite3 *db)
{
    return SQL_TotalChanges(db);
}

sqlite3_int64 sqlite3_total_changes64(sqlite3 *db)
{
    sqlite3_int64 lib_result = 0;
    SQL_TotalChanges64(db, &lib_result);
    return lib_result;
}

void sqlite3_interrupt(sqlite3 *db)
{
    SQL_Interrupt(db);
}

int sqlite3_is_interrupted(sqlite3 *db)
{
    return SQL_IsInterrupted(db);
}

int sqlite3_complete(const char *sql)
{
    return SQL_Complete(sql);
}

int sqlite3_busy_handler(sqlite3 *db, int (*xBusy)(void*,int), void *arg)
{
    return SQL_BusyHandler(db, (APTR)xBusy, arg);
}

int sqlite3_busy_timeout(sqlite3 *db, int ms)
{
    return SQL_BusyTimeout(db, ms);
}

void *sqlite3_malloc(int n)
{
    return SQL_Malloc(n);
}

void *sqlite3_malloc64(sqlite3_uint64 n)
{
    return SQL_Malloc64((ULONG)((sqlite3_uint64)n >> 32), (ULONG)n);
}

void *sqlite3_realloc(void *p, int n)
{
    return SQL_Realloc(p, n);
}

void *sqlite3_realloc64(void *p, sqlite3_uint64 n)
{
    return SQL_Realloc64(p, (ULONG)((sqlite3_uint64)n >> 32), (ULONG)n);
}

void sqlite3_free(void *p)
{
    SQL_Free(p);
}

sqlite3_uint64 sqlite3_msize(void *p)
{
    sqlite3_uint64 lib_result = 0;
    SQL_Msize(p, &lib_result);
    return lib_result;
}

sqlite3_int64 sqlite3_memory_used(void)
{
    sqlite3_int64 lib_result = 0;
    SQL_MemoryUsed(&lib_result);
    return lib_result;
}

sqlite3_int64 sqlite3_memory_highwater(int resetFlag)
{
    sqlite3_int64 lib_result = 0;
    SQL_MemoryHighwater(resetFlag, &lib_result);
    return lib_result;
}

void sqlite3_randomness(int N, void *P)
{
    SQL_Randomness(N, P);
}

int sqlite3_set_authorizer(sqlite3 *db, int (*xAuth)(void*,int,const char*,const char*,const char*,const char*), void *pUserData)
{
    return SQL_SetAuthorizer(db, (APTR)xAuth, pUserData);
}

void sqlite3_progress_handler(sqlite3 *db, int nOps, int (*xProgress)(void*), void *pArg)
{
    SQL_ProgressHandler(db, nOps, (APTR)xProgress, pArg);
}

int sqlite3_errcode(sqlite3 *db)
{
    return SQL_Errcode(db);
}

int sqlite3_extended_errcode(sqlite3 *db)
{
    return SQL_ExtendedErrcode(db);
}

const char *sqlite3_errmsg(sqlite3 *db)
{
    return SQL_Errmsg(db);
}

const char *sqlite3_errstr(int rc)
{
    return SQL_Errstr(rc);
}

int sqlite3_error_offset(sqlite3 *db)
{
    return SQL_ErrorOffset(db);
}

int sqlite3_system_errno(sqlite3 *db)
{
    return SQL_SystemErrno(db);
}

int sqlite3_limit(sqlite3 *db, int id, int newVal)
{
    return SQL_Limit(db, id, newVal);
}

int sqlite3_prepare_v2(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail)
{
    return SQL_PrepareV2(db, zSql, nByte, ppStmt, pzTail);
}

int sqlite3_prepare_v3(sqlite3 *db, const char *zSql, int nByte, unsigned int prepFlags, sqlite3_stmt **ppStmt, const char **pzTail)
{
    return SQL_PrepareV3(db, zSql, nByte, prepFlags, ppStmt, pzTail);
}

const char *sqlite3_sql(sqlite3_stmt *pStmt)
{
    return SQL_Sql(pStmt);
}

char *sqlite3_expanded_sql(sqlite3_stmt *pStmt)
{
    return SQL_ExpandedSql(pStmt);
}

int sqlite3_stmt_readonly(sqlite3_stmt *pStmt)
{
    return SQL_StmtReadonly(pStmt);
}

int sqlite3_stmt_isexplain(sqlite3_stmt *pStmt)
{
    return SQL_StmtIsexplain(pStmt);
}

int sqlite3_stmt_busy(sqlite3_stmt *pStmt)
{
    return SQL_StmtBusy(pStmt);
}

int sqlite3_stmt_status(sqlite3_stmt *pStmt, int op, int resetFlg)
{
    return SQL_StmtStatus(pStmt, op, resetFlg);
}

int sqlite3_bind_blob(sqlite3_stmt *pStmt, int idx, const void *data, int n, void (*xDel)(void*))
{
    return SQL_BindBlob(pStmt, idx, data, n, (APTR)xDel);
}

int sqlite3_bind_blob64(sqlite3_stmt *pStmt, int idx, const void *data, sqlite3_uint64 n, void (*xDel)(void*))
{
    return SQL_BindBlob64(pStmt, idx, data, (ULONG)((sqlite3_uint64)n >> 32), (ULONG)n, (APTR)xDel);
}

int sqlite3_bind_double(sqlite3_stmt *pStmt, int idx, double value)
{
    return SQL_BindDouble(pStmt, idx, &value);
}

int sqlite3_bind_int(sqlite3_stmt *pStmt, int idx, int value)
{
    return SQL_BindInt(pStmt, idx, value);
}

int sqlite3_bind_int64(sqlite3_stmt *pStmt, int idx, sqlite3_int64 value)
{
    return SQL_BindInt64(pStmt, idx, (ULONG)((sqlite3_uint64)value >> 32), (ULONG)value);
}

int sqlite3_bind_null(sqlite3_stmt *pStmt, int idx)
{
    return SQL_BindNull(pStmt, idx);
}

int sqlite3_bind_text(sqlite3_stmt *pStmt, int idx, const char *data, int n, void (*xDel)(void*))
{
    return SQL_BindText(pStmt, idx, data, n, (APTR)xDel);
}

int sqlite3_bind_text64(sqlite3_stmt *pStmt, int idx, const char *data, sqlite3_uint64 n, void (*xDel)(void*), unsigned char encoding)
{
    return SQL_BindText64(pStmt, idx, data, (ULONG)((sqlite3_uint64)n >> 32), (ULONG)n, (APTR)xDel, encoding);
}

int sqlite3_bind_value(sqlite3_stmt *pStmt, int idx, const sqlite3_value *value)
{
    return SQL_BindValue(pStmt, idx, value);
}

int sqlite3_bind_zeroblob(sqlite3_stmt *pStmt, int idx, int n)
{
    return SQL_BindZeroblob(pStmt, idx, n);
}

int sqlite3_bind_parameter_count(sqlite3_stmt *pStmt)
{
    return SQL_BindParameterCount(pStmt);
}

const char *sqlite3_bind_parameter_name(sqlite3_stmt *pStmt, int idx)
{
    return SQL_BindParameterName(pStmt, idx);
}

int sqlite3_bind_parameter_index(sqlite3_stmt *pStmt, const char *zName)
{
    return SQL_BindParameterIndex(pStmt, zName);
}

int sqlite3_clear_bindings(sqlite3_stmt *pStmt)
{
    return SQL_ClearBindings(pStmt);
}

int sqlite3_column_count(sqlite3_stmt *pStmt)
{
    return SQL_ColumnCount(pStmt);
}

const char *sqlite3_column_name(sqlite3_stmt *pStmt, int N)
{
    return SQL_ColumnName(pStmt, N);
}

const char *sqlite3_column_decltype(sqlite3_stmt *pStmt, int N)
{
    return SQL_ColumnDecltype(pStmt, N);
}

int sqlite3_step(sqlite3_stmt *pStmt)
{
    return SQL_Step(pStmt);
}

int sqlite3_data_count(sqlite3_stmt *pStmt)
{
    return SQL_DataCount(pStmt);
}

const void *sqlite3_column_blob(sqlite3_stmt *pStmt, int iCol)
{
    return SQL_ColumnBlob(pStmt, iCol);
}

double sqlite3_column_double(sqlite3_stmt *pStmt, int iCol)
{
    double lib_result = 0;
    SQL_ColumnDouble(pStmt, iCol, &lib_result);
    return lib_result;
}

int sqlite3_column_int(sqlite3_stmt *pStmt, int iCol)
{
    return SQL_ColumnInt(pStmt, iCol);
}

sqlite3_int64 sqlite3_column_int64(sqlite3_stmt *pStmt, int iCol)
{
    sqlite3_int64 lib_result = 0;
    SQL_ColumnInt64(pStmt, iCol, &lib_result);
    return lib_result;
}

const unsigned char *sqlite3_column_text(sqlite3_stmt *pStmt, int iCol)
{
    return SQL_ColumnText(pStmt, iCol);
}

sqlite3_value *sqlite3_column_value(sqlite3_stmt *pStmt, int iCol)
{
    return SQL_ColumnValue(pStmt, iCol);
}

int sqlite3_column_bytes(sqlite3_stmt *pStmt, int iCol)
{
    return SQL_ColumnBytes(pStmt, iCol);
}

int sqlite3_column_type(sqlite3_stmt *pStmt, int iCol)
{
    return SQL_ColumnType(pStmt, iCol);
}

int sqlite3_finalize(sqlite3_stmt *pStmt)
{
    return SQL_Finalize(pStmt);
}

int sqlite3_reset(sqlite3_stmt *pStmt)
{
    return SQL_Reset(pStmt);
}

int sqlite3_create_function(sqlite3 *db, const char *zFunctionName, int nArg, int eTextRep, void *pApp, void (*xFunc)(sqlite3_context*,int,sqlite3_value**), void (*xStep)(sqlite3_context*,int,sqlite3_value**), void (*xFinal)(sqlite3_context*))
{
    return SQL_CreateFunction(db, zFunctionName, nArg, eTextRep, pApp, (APTR)xFunc, (APTR)xStep, (APTR)xFinal);
}

int sqlite3_create_function_v2(sqlite3 *db, const char *zFunctionName, int nArg, int eTextRep, void *pApp, void (*xFunc)(sqlite3_context*,int,sqlite3_value**), void (*xStep)(sqlite3_context*,int,sqlite3_value**), void (*xFinal)(sqlite3_context*), void (*xDestroy)(void*))
{
    return SQL_CreateFunctionV2(db, zFunctionName, nArg, eTextRep, pApp, (APTR)xFunc, (APTR)xStep, (APTR)xFinal, (APTR)xDestroy);
}

const void *sqlite3_value_blob(sqlite3_value *pVal)
{
    return SQL_ValueBlob(pVal);
}

double sqlite3_value_double(sqlite3_value *pVal)
{
    double lib_result = 0;
    SQL_ValueDouble(pVal, &lib_result);
    return lib_result;
}

int sqlite3_value_int(sqlite3_value *pVal)
{
    return SQL_ValueInt(pVal);
}

sqlite3_int64 sqlite3_value_int64(sqlite3_value *pVal)
{
    sqlite3_int64 lib_result = 0;
    SQL_ValueInt64(pVal, &lib_result);
    return lib_result;
}

const unsigned char *sqlite3_value_text(sqlite3_value *pVal)
{
    return SQL_ValueText(pVal);
}

int sqlite3_value_bytes(sqlite3_value *pVal)
{
    return SQL_ValueBytes(pVal);
}

int sqlite3_value_type(sqlite3_value *pVal)
{
    return SQL_ValueType(pVal);
}

int sqlite3_value_numeric_type(sqlite3_value *pVal)
{
    return SQL_ValueNumericType(pVal);
}

sqlite3_value *sqlite3_value_dup(const sqlite3_value *pVal)
{
    return SQL_ValueDup(pVal);
}

void sqlite3_value_free(sqlite3_value *pVal)
{
    SQL_ValueFree(pVal);
}

void *sqlite3_aggregate_context(sqlite3_context *ctx, int nBytes)
{
    return SQL_AggregateContext(ctx, nBytes);
}

void *sqlite3_user_data(sqlite3_context *ctx)
{
    return SQL_UserData(ctx);
}

sqlite3 *sqlite3_context_db_handle(sqlite3_context *ctx)
{
    return SQL_ContextDbHandle(ctx);
}

void sqlite3_result_blob(sqlite3_context *ctx, const void *data, int n, void (*xDel)(void*))
{
    SQL_ResultBlob(ctx, data, n, (APTR)xDel);
}

void sqlite3_result_double(sqlite3_context *ctx, double value)
{
    SQL_ResultDouble(ctx, &value);
}

void sqlite3_result_error(sqlite3_context *ctx, const char *msg, int n)
{
    SQL_ResultError(ctx, msg, n);
}

void sqlite3_result_error_toobig(sqlite3_context *ctx)
{
    SQL_ResultErrorToobig(ctx);
}

void sqlite3_result_error_nomem(sqlite3_context *ctx)
{
    SQL_ResultErrorNomem(ctx);
}

void sqlite3_result_error_code(sqlite3_context *ctx, int code)
{
    SQL_ResultErrorCode(ctx, code);
}

void sqlite3_result_int(sqlite3_context *ctx, int value)
{
    SQL_ResultInt(ctx, value);
}

void sqlite3_result_int64(sqlite3_context *ctx, sqlite3_int64 value)
{
    SQL_ResultInt64(ctx, (ULONG)((sqlite3_uint64)value >> 32), (ULONG)value);
}

void sqlite3_result_null(sqlite3_context *ctx)
{
    SQL_ResultNull(ctx);
}

void sqlite3_result_text(sqlite3_context *ctx, const char *data, int n, void (*xDel)(void*))
{
    SQL_ResultText(ctx, data, n, (APTR)xDel);
}

void sqlite3_result_value(sqlite3_context *ctx, sqlite3_value *value)
{
    SQL_ResultValue(ctx, value);
}

void sqlite3_result_zeroblob(sqlite3_context *ctx, int n)
{
    SQL_ResultZeroblob(ctx, n);
}

int sqlite3_create_collation(sqlite3 *db, const char *zName, int eTextRep, void *pArg, int (*xCompare)(void*,int,const void*,int,const void*))
{
    return SQL_CreateCollation(db, zName, eTextRep, pArg, (APTR)xCompare);
}

int sqlite3_create_collation_v2(sqlite3 *db, const char *zName, int eTextRep, void *pArg, int (*xCompare)(void*,int,const void*,int,const void*), void (*xDestroy)(void*))
{
    return SQL_CreateCollationV2(db, zName, eTextRep, pArg, (APTR)xCompare, (APTR)xDestroy);
}

int sqlite3_get_autocommit(sqlite3 *db)
{
    return SQL_GetAutocommit(db);
}

int sqlite3_txn_state(sqlite3 *db, const char *zSchema)
{
    return SQL_TxnState(db, zSchema);
}

sqlite3 *sqlite3_db_handle(sqlite3_stmt *pStmt)
{
    return SQL_DbHandle(pStmt);
}

const char *sqlite3_db_filename(sqlite3 *db, const char *zDbName)
{
    return SQL_DbFilename(db, zDbName);
}

int sqlite3_db_readonly(sqlite3 *db, const char *zDbName)
{
    return SQL_DbReadonly(db, zDbName);
}

sqlite3_stmt *sqlite3_next_stmt(sqlite3 *db, sqlite3_stmt *pStmt)
{
    return SQL_NextStmt(db, pStmt);
}

void *sqlite3_commit_hook(sqlite3 *db, int (*xCallback)(void*), void *pArg)
{
    return SQL_CommitHook(db, (APTR)xCallback, pArg);
}

void *sqlite3_rollback_hook(sqlite3 *db, void (*xCallback)(void*), void *pArg)
{
    return SQL_RollbackHook(db, (APTR)xCallback, pArg);
}

void *sqlite3_update_hook(sqlite3 *db, void (*xCallback)(void*,int,const char*,const char*,sqlite3_int64), void *pArg)
{
    return SQL_UpdateHook(db, (APTR)xCallback, pArg);
}

int sqlite3_trace_v2(sqlite3 *db, unsigned int uMask, int (*xCallback)(unsigned int,void*,void*,void*), void *pCtx)
{
    return SQL_TraceV2(db, uMask, (APTR)xCallback, pCtx);
}

int sqlite3_db_release_memory(sqlite3 *db)
{
    return SQL_DbReleaseMemory(db);
}

int sqlite3_db_cacheflush(sqlite3 *db)
{
    return SQL_DbCacheflush(db);
}

int sqlite3_file_control(sqlite3 *db, const char *zDbName, int op, void *pArg)
{
    return SQL_FileControl(db, zDbName, op, pArg);
}

void *sqlite3_wal_hook(sqlite3 *db, int (*xCallback)(void*,sqlite3*,const char*,int), void *pArg)
{
    return SQL_WalHook(db, (APTR)xCallback, pArg);
}

int sqlite3_wal_checkpoint_v2(sqlite3 *db, const char *zDb, int eMode, int *pnLog, int *pnCkpt)
{
    return SQL_WalCheckpointV2(db, zDb, eMode, pnLog, pnCkpt);
}

sqlite3_str *sqlite3_str_new(sqlite3 *db)
{
    return SQL_StrNew(db);
}

char *sqlite3_str_finish(sqlite3_str *p)
{
    return SQL_StrFinish(p);
}

void sqlite3_str_vappendf(sqlite3_str *p, const char *zFormat, va_list ap)
{
    SQL_StrVappendf(p, zFormat, ap);
}

void sqlite3_str_append(sqlite3_str *p, const char *zIn, int N)
{
    SQL_StrAppend(p, zIn, N);
}

void sqlite3_str_appendall(sqlite3_str *p, const char *zIn)
{
    SQL_StrAppendall(p, zIn);
}

void sqlite3_str_appendchar(sqlite3_str *p, int N, char C)
{
    SQL_StrAppendchar(p, N, C);
}

void sqlite3_str_reset(sqlite3_str *p)
{
    SQL_StrReset(p);
}

int sqlite3_str_errcode(sqlite3_str *p)
{
    return SQL_StrErrcode(p);
}

int sqlite3_str_length(sqlite3_str *p)
{
    return SQL_StrLength(p);
}

char *sqlite3_str_value(sqlite3_str *p)
{
    return SQL_StrValue(p);
}

char *sqlite3_vmprintf(const char *zFormat, va_list ap)
{
    return SQL_Vmprintf(zFormat, ap);
}

char *sqlite3_vsnprintf(int n, char *zBuf, const char *zFormat, va_list ap)
{
    return SQL_Vsnprintf(n, zBuf, zFormat, ap);
}

int sqlite3_stricmp(const char *a, const char *b)
{
    return SQL_Stricmp(a, b);
}

int sqlite3_strnicmp(const char *a, const char *b, int n)
{
    return SQL_Strnicmp(a, b, n);
}

int sqlite3_strglob(const char *zGlob, const char *zStr)
{
    return SQL_Strglob(zGlob, zStr);
}

int sqlite3_strlike(const char *zGlob, const char *zStr, unsigned int cEsc)
{
    return SQL_Strlike(zGlob, zStr, cEsc);
}

int sqlite3_sleep(int ms)
{
    return SQL_Sleep(ms);
}

sqlite3_vfs *sqlite3_vfs_find(const char *zVfsName)
{
    return SQL_VfsFind(zVfsName);
}
