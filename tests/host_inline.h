/* Made by tools/gen-library.py from its table: don't edit. sqlite.library's calls as
 * plain C calls into the library's side, for the test that runs the link
 * library on this computer (tests/host-libtest.sh).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef SQLITE_HOST_INLINE_H
#define SQLITE_HOST_INLINE_H

void * L_SQL_Attach(void);
#define SQL_Attach() L_SQL_Attach()
void L_SQL_Detach(void *owner);
#define SQL_Detach(owner) L_SQL_Detach(owner)
int L_SQL_DbConfig(sqlite3 *db, int op, void *p, int a, int b);
#define SQL_DbConfig(db,op,p,a,b) L_SQL_DbConfig(db,op,p,a,b)
const char * L_SQL_Libversion(void);
#define SQL_Libversion() L_SQL_Libversion()
const char * L_SQL_Sourceid(void);
#define SQL_Sourceid() L_SQL_Sourceid()
int L_SQL_LibversionNumber(void);
#define SQL_LibversionNumber() L_SQL_LibversionNumber()
int L_SQL_Threadsafe(void);
#define SQL_Threadsafe() L_SQL_Threadsafe()
int L_SQL_Open(const char *filename, sqlite3 **ppDb, APTR owner);
#define SQL_Open(filename,ppDb,owner) L_SQL_Open(filename,ppDb,owner)
int L_SQL_OpenV2(const char *filename, sqlite3 **ppDb, int flags, const char *zVfs, APTR owner);
#define SQL_OpenV2(filename,ppDb,flags,zVfs,owner) L_SQL_OpenV2(filename,ppDb,flags,zVfs,owner)
int L_SQL_Close(sqlite3 *db);
#define SQL_Close(db) L_SQL_Close(db)
int L_SQL_CloseV2(sqlite3 *db);
#define SQL_CloseV2(db) L_SQL_CloseV2(db)
int L_SQL_Exec(sqlite3 *db, const char *sql, APTR callback, void *arg, char **errmsg);
#define SQL_Exec(db,sql,callback,arg,errmsg) L_SQL_Exec(db,sql,callback,arg,errmsg)
int L_SQL_ExtendedResultCodes(sqlite3 *db, int onoff);
#define SQL_ExtendedResultCodes(db,onoff) L_SQL_ExtendedResultCodes(db,onoff)
void L_SQL_LastInsertRowid(sqlite3 *db, sqlite3_int64 *lib_result);
#define SQL_LastInsertRowid(db,lib_result) L_SQL_LastInsertRowid(db,lib_result)
void L_SQL_SetLastInsertRowid(sqlite3 *db, ULONG rowid_hi, ULONG rowid_lo);
#define SQL_SetLastInsertRowid(db,rowid_hi,rowid_lo) L_SQL_SetLastInsertRowid(db,rowid_hi,rowid_lo)
int L_SQL_Changes(sqlite3 *db);
#define SQL_Changes(db) L_SQL_Changes(db)
void L_SQL_Changes64(sqlite3 *db, sqlite3_int64 *lib_result);
#define SQL_Changes64(db,lib_result) L_SQL_Changes64(db,lib_result)
int L_SQL_TotalChanges(sqlite3 *db);
#define SQL_TotalChanges(db) L_SQL_TotalChanges(db)
void L_SQL_TotalChanges64(sqlite3 *db, sqlite3_int64 *lib_result);
#define SQL_TotalChanges64(db,lib_result) L_SQL_TotalChanges64(db,lib_result)
void L_SQL_Interrupt(sqlite3 *db);
#define SQL_Interrupt(db) L_SQL_Interrupt(db)
int L_SQL_IsInterrupted(sqlite3 *db);
#define SQL_IsInterrupted(db) L_SQL_IsInterrupted(db)
int L_SQL_Complete(const char *sql);
#define SQL_Complete(sql) L_SQL_Complete(sql)
int L_SQL_BusyHandler(sqlite3 *db, APTR xBusy, void *arg);
#define SQL_BusyHandler(db,xBusy,arg) L_SQL_BusyHandler(db,xBusy,arg)
int L_SQL_BusyTimeout(sqlite3 *db, int ms);
#define SQL_BusyTimeout(db,ms) L_SQL_BusyTimeout(db,ms)
void * L_SQL_Malloc(int n);
#define SQL_Malloc(n) L_SQL_Malloc(n)
void * L_SQL_Malloc64(ULONG n_hi, ULONG n_lo);
#define SQL_Malloc64(n_hi,n_lo) L_SQL_Malloc64(n_hi,n_lo)
void * L_SQL_Realloc(void *p, int n);
#define SQL_Realloc(p,n) L_SQL_Realloc(p,n)
void * L_SQL_Realloc64(void *p, ULONG n_hi, ULONG n_lo);
#define SQL_Realloc64(p,n_hi,n_lo) L_SQL_Realloc64(p,n_hi,n_lo)
void L_SQL_Free(void *p);
#define SQL_Free(p) L_SQL_Free(p)
void L_SQL_Msize(void *p, sqlite3_uint64 *lib_result);
#define SQL_Msize(p,lib_result) L_SQL_Msize(p,lib_result)
void L_SQL_MemoryUsed(sqlite3_int64 *lib_result);
#define SQL_MemoryUsed(lib_result) L_SQL_MemoryUsed(lib_result)
void L_SQL_MemoryHighwater(int resetFlag, sqlite3_int64 *lib_result);
#define SQL_MemoryHighwater(resetFlag,lib_result) L_SQL_MemoryHighwater(resetFlag,lib_result)
void L_SQL_Randomness(int N, void *P);
#define SQL_Randomness(N,P) L_SQL_Randomness(N,P)
int L_SQL_SetAuthorizer(sqlite3 *db, APTR xAuth, void *pUserData);
#define SQL_SetAuthorizer(db,xAuth,pUserData) L_SQL_SetAuthorizer(db,xAuth,pUserData)
void L_SQL_ProgressHandler(sqlite3 *db, int nOps, APTR xProgress, void *pArg);
#define SQL_ProgressHandler(db,nOps,xProgress,pArg) L_SQL_ProgressHandler(db,nOps,xProgress,pArg)
int L_SQL_Errcode(sqlite3 *db);
#define SQL_Errcode(db) L_SQL_Errcode(db)
int L_SQL_ExtendedErrcode(sqlite3 *db);
#define SQL_ExtendedErrcode(db) L_SQL_ExtendedErrcode(db)
const char * L_SQL_Errmsg(sqlite3 *db);
#define SQL_Errmsg(db) L_SQL_Errmsg(db)
const char * L_SQL_Errstr(int rc);
#define SQL_Errstr(rc) L_SQL_Errstr(rc)
int L_SQL_ErrorOffset(sqlite3 *db);
#define SQL_ErrorOffset(db) L_SQL_ErrorOffset(db)
int L_SQL_SystemErrno(sqlite3 *db);
#define SQL_SystemErrno(db) L_SQL_SystemErrno(db)
int L_SQL_Limit(sqlite3 *db, int id, int newVal);
#define SQL_Limit(db,id,newVal) L_SQL_Limit(db,id,newVal)
int L_SQL_PrepareV2(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
#define SQL_PrepareV2(db,zSql,nByte,ppStmt,pzTail) L_SQL_PrepareV2(db,zSql,nByte,ppStmt,pzTail)
int L_SQL_PrepareV3(sqlite3 *db, const char *zSql, int nByte, unsigned int prepFlags, sqlite3_stmt **ppStmt, const char **pzTail);
#define SQL_PrepareV3(db,zSql,nByte,prepFlags,ppStmt,pzTail) L_SQL_PrepareV3(db,zSql,nByte,prepFlags,ppStmt,pzTail)
const char * L_SQL_Sql(sqlite3_stmt *pStmt);
#define SQL_Sql(pStmt) L_SQL_Sql(pStmt)
char * L_SQL_ExpandedSql(sqlite3_stmt *pStmt);
#define SQL_ExpandedSql(pStmt) L_SQL_ExpandedSql(pStmt)
int L_SQL_StmtReadonly(sqlite3_stmt *pStmt);
#define SQL_StmtReadonly(pStmt) L_SQL_StmtReadonly(pStmt)
int L_SQL_StmtIsexplain(sqlite3_stmt *pStmt);
#define SQL_StmtIsexplain(pStmt) L_SQL_StmtIsexplain(pStmt)
int L_SQL_StmtBusy(sqlite3_stmt *pStmt);
#define SQL_StmtBusy(pStmt) L_SQL_StmtBusy(pStmt)
int L_SQL_StmtStatus(sqlite3_stmt *pStmt, int op, int resetFlg);
#define SQL_StmtStatus(pStmt,op,resetFlg) L_SQL_StmtStatus(pStmt,op,resetFlg)
int L_SQL_BindBlob(sqlite3_stmt *pStmt, int idx, const void *data, int n, APTR xDel);
#define SQL_BindBlob(pStmt,idx,data,n,xDel) L_SQL_BindBlob(pStmt,idx,data,n,xDel)
int L_SQL_BindBlob64(sqlite3_stmt *pStmt, int idx, const void *data, ULONG n_hi, ULONG n_lo, APTR xDel);
#define SQL_BindBlob64(pStmt,idx,data,n_hi,n_lo,xDel) L_SQL_BindBlob64(pStmt,idx,data,n_hi,n_lo,xDel)
int L_SQL_BindDouble(sqlite3_stmt *pStmt, int idx, const double *value_p);
#define SQL_BindDouble(pStmt,idx,value_p) L_SQL_BindDouble(pStmt,idx,value_p)
int L_SQL_BindInt(sqlite3_stmt *pStmt, int idx, int value);
#define SQL_BindInt(pStmt,idx,value) L_SQL_BindInt(pStmt,idx,value)
int L_SQL_BindInt64(sqlite3_stmt *pStmt, int idx, ULONG value_hi, ULONG value_lo);
#define SQL_BindInt64(pStmt,idx,value_hi,value_lo) L_SQL_BindInt64(pStmt,idx,value_hi,value_lo)
int L_SQL_BindNull(sqlite3_stmt *pStmt, int idx);
#define SQL_BindNull(pStmt,idx) L_SQL_BindNull(pStmt,idx)
int L_SQL_BindText(sqlite3_stmt *pStmt, int idx, const char *data, int n, APTR xDel);
#define SQL_BindText(pStmt,idx,data,n,xDel) L_SQL_BindText(pStmt,idx,data,n,xDel)
int L_SQL_BindText64(sqlite3_stmt *pStmt, int idx, const char *data, ULONG n_hi, ULONG n_lo, APTR xDel, unsigned char encoding);
#define SQL_BindText64(pStmt,idx,data,n_hi,n_lo,xDel,encoding) L_SQL_BindText64(pStmt,idx,data,n_hi,n_lo,xDel,encoding)
int L_SQL_BindValue(sqlite3_stmt *pStmt, int idx, const sqlite3_value *value);
#define SQL_BindValue(pStmt,idx,value) L_SQL_BindValue(pStmt,idx,value)
int L_SQL_BindZeroblob(sqlite3_stmt *pStmt, int idx, int n);
#define SQL_BindZeroblob(pStmt,idx,n) L_SQL_BindZeroblob(pStmt,idx,n)
int L_SQL_BindParameterCount(sqlite3_stmt *pStmt);
#define SQL_BindParameterCount(pStmt) L_SQL_BindParameterCount(pStmt)
const char * L_SQL_BindParameterName(sqlite3_stmt *pStmt, int idx);
#define SQL_BindParameterName(pStmt,idx) L_SQL_BindParameterName(pStmt,idx)
int L_SQL_BindParameterIndex(sqlite3_stmt *pStmt, const char *zName);
#define SQL_BindParameterIndex(pStmt,zName) L_SQL_BindParameterIndex(pStmt,zName)
int L_SQL_ClearBindings(sqlite3_stmt *pStmt);
#define SQL_ClearBindings(pStmt) L_SQL_ClearBindings(pStmt)
int L_SQL_ColumnCount(sqlite3_stmt *pStmt);
#define SQL_ColumnCount(pStmt) L_SQL_ColumnCount(pStmt)
const char * L_SQL_ColumnName(sqlite3_stmt *pStmt, int N);
#define SQL_ColumnName(pStmt,N) L_SQL_ColumnName(pStmt,N)
const char * L_SQL_ColumnDecltype(sqlite3_stmt *pStmt, int N);
#define SQL_ColumnDecltype(pStmt,N) L_SQL_ColumnDecltype(pStmt,N)
int L_SQL_Step(sqlite3_stmt *pStmt);
#define SQL_Step(pStmt) L_SQL_Step(pStmt)
int L_SQL_DataCount(sqlite3_stmt *pStmt);
#define SQL_DataCount(pStmt) L_SQL_DataCount(pStmt)
const void * L_SQL_ColumnBlob(sqlite3_stmt *pStmt, int iCol);
#define SQL_ColumnBlob(pStmt,iCol) L_SQL_ColumnBlob(pStmt,iCol)
void L_SQL_ColumnDouble(sqlite3_stmt *pStmt, int iCol, double *lib_result);
#define SQL_ColumnDouble(pStmt,iCol,lib_result) L_SQL_ColumnDouble(pStmt,iCol,lib_result)
int L_SQL_ColumnInt(sqlite3_stmt *pStmt, int iCol);
#define SQL_ColumnInt(pStmt,iCol) L_SQL_ColumnInt(pStmt,iCol)
void L_SQL_ColumnInt64(sqlite3_stmt *pStmt, int iCol, sqlite3_int64 *lib_result);
#define SQL_ColumnInt64(pStmt,iCol,lib_result) L_SQL_ColumnInt64(pStmt,iCol,lib_result)
const unsigned char * L_SQL_ColumnText(sqlite3_stmt *pStmt, int iCol);
#define SQL_ColumnText(pStmt,iCol) L_SQL_ColumnText(pStmt,iCol)
sqlite3_value * L_SQL_ColumnValue(sqlite3_stmt *pStmt, int iCol);
#define SQL_ColumnValue(pStmt,iCol) L_SQL_ColumnValue(pStmt,iCol)
int L_SQL_ColumnBytes(sqlite3_stmt *pStmt, int iCol);
#define SQL_ColumnBytes(pStmt,iCol) L_SQL_ColumnBytes(pStmt,iCol)
int L_SQL_ColumnType(sqlite3_stmt *pStmt, int iCol);
#define SQL_ColumnType(pStmt,iCol) L_SQL_ColumnType(pStmt,iCol)
int L_SQL_Finalize(sqlite3_stmt *pStmt);
#define SQL_Finalize(pStmt) L_SQL_Finalize(pStmt)
int L_SQL_Reset(sqlite3_stmt *pStmt);
#define SQL_Reset(pStmt) L_SQL_Reset(pStmt)
int L_SQL_CreateFunction(sqlite3 *db, const char *zFunctionName, int nArg, int eTextRep, void *pApp, APTR xFunc, APTR xStep, APTR xFinal);
#define SQL_CreateFunction(db,zFunctionName,nArg,eTextRep,pApp,xFunc,xStep,xFinal) L_SQL_CreateFunction(db,zFunctionName,nArg,eTextRep,pApp,xFunc,xStep,xFinal)
int L_SQL_CreateFunctionV2(sqlite3 *db, const char *zFunctionName, int nArg, int eTextRep, void *pApp, APTR xFunc, APTR xStep, APTR xFinal, APTR xDestroy);
#define SQL_CreateFunctionV2(db,zFunctionName,nArg,eTextRep,pApp,xFunc,xStep,xFinal,xDestroy) L_SQL_CreateFunctionV2(db,zFunctionName,nArg,eTextRep,pApp,xFunc,xStep,xFinal,xDestroy)
const void * L_SQL_ValueBlob(sqlite3_value *pVal);
#define SQL_ValueBlob(pVal) L_SQL_ValueBlob(pVal)
void L_SQL_ValueDouble(sqlite3_value *pVal, double *lib_result);
#define SQL_ValueDouble(pVal,lib_result) L_SQL_ValueDouble(pVal,lib_result)
int L_SQL_ValueInt(sqlite3_value *pVal);
#define SQL_ValueInt(pVal) L_SQL_ValueInt(pVal)
void L_SQL_ValueInt64(sqlite3_value *pVal, sqlite3_int64 *lib_result);
#define SQL_ValueInt64(pVal,lib_result) L_SQL_ValueInt64(pVal,lib_result)
const unsigned char * L_SQL_ValueText(sqlite3_value *pVal);
#define SQL_ValueText(pVal) L_SQL_ValueText(pVal)
int L_SQL_ValueBytes(sqlite3_value *pVal);
#define SQL_ValueBytes(pVal) L_SQL_ValueBytes(pVal)
int L_SQL_ValueType(sqlite3_value *pVal);
#define SQL_ValueType(pVal) L_SQL_ValueType(pVal)
int L_SQL_ValueNumericType(sqlite3_value *pVal);
#define SQL_ValueNumericType(pVal) L_SQL_ValueNumericType(pVal)
sqlite3_value * L_SQL_ValueDup(const sqlite3_value *pVal);
#define SQL_ValueDup(pVal) L_SQL_ValueDup(pVal)
void L_SQL_ValueFree(sqlite3_value *pVal);
#define SQL_ValueFree(pVal) L_SQL_ValueFree(pVal)
void * L_SQL_AggregateContext(sqlite3_context *ctx, int nBytes);
#define SQL_AggregateContext(ctx,nBytes) L_SQL_AggregateContext(ctx,nBytes)
void * L_SQL_UserData(sqlite3_context *ctx);
#define SQL_UserData(ctx) L_SQL_UserData(ctx)
sqlite3 * L_SQL_ContextDbHandle(sqlite3_context *ctx);
#define SQL_ContextDbHandle(ctx) L_SQL_ContextDbHandle(ctx)
void L_SQL_ResultBlob(sqlite3_context *ctx, const void *data, int n, APTR xDel);
#define SQL_ResultBlob(ctx,data,n,xDel) L_SQL_ResultBlob(ctx,data,n,xDel)
void L_SQL_ResultDouble(sqlite3_context *ctx, const double *value_p);
#define SQL_ResultDouble(ctx,value_p) L_SQL_ResultDouble(ctx,value_p)
void L_SQL_ResultError(sqlite3_context *ctx, const char *msg, int n);
#define SQL_ResultError(ctx,msg,n) L_SQL_ResultError(ctx,msg,n)
void L_SQL_ResultErrorToobig(sqlite3_context *ctx);
#define SQL_ResultErrorToobig(ctx) L_SQL_ResultErrorToobig(ctx)
void L_SQL_ResultErrorNomem(sqlite3_context *ctx);
#define SQL_ResultErrorNomem(ctx) L_SQL_ResultErrorNomem(ctx)
void L_SQL_ResultErrorCode(sqlite3_context *ctx, int code);
#define SQL_ResultErrorCode(ctx,code) L_SQL_ResultErrorCode(ctx,code)
void L_SQL_ResultInt(sqlite3_context *ctx, int value);
#define SQL_ResultInt(ctx,value) L_SQL_ResultInt(ctx,value)
void L_SQL_ResultInt64(sqlite3_context *ctx, ULONG value_hi, ULONG value_lo);
#define SQL_ResultInt64(ctx,value_hi,value_lo) L_SQL_ResultInt64(ctx,value_hi,value_lo)
void L_SQL_ResultNull(sqlite3_context *ctx);
#define SQL_ResultNull(ctx) L_SQL_ResultNull(ctx)
void L_SQL_ResultText(sqlite3_context *ctx, const char *data, int n, APTR xDel);
#define SQL_ResultText(ctx,data,n,xDel) L_SQL_ResultText(ctx,data,n,xDel)
void L_SQL_ResultValue(sqlite3_context *ctx, sqlite3_value *value);
#define SQL_ResultValue(ctx,value) L_SQL_ResultValue(ctx,value)
void L_SQL_ResultZeroblob(sqlite3_context *ctx, int n);
#define SQL_ResultZeroblob(ctx,n) L_SQL_ResultZeroblob(ctx,n)
int L_SQL_CreateCollation(sqlite3 *db, const char *zName, int eTextRep, void *pArg, APTR xCompare);
#define SQL_CreateCollation(db,zName,eTextRep,pArg,xCompare) L_SQL_CreateCollation(db,zName,eTextRep,pArg,xCompare)
int L_SQL_CreateCollationV2(sqlite3 *db, const char *zName, int eTextRep, void *pArg, APTR xCompare, APTR xDestroy);
#define SQL_CreateCollationV2(db,zName,eTextRep,pArg,xCompare,xDestroy) L_SQL_CreateCollationV2(db,zName,eTextRep,pArg,xCompare,xDestroy)
int L_SQL_GetAutocommit(sqlite3 *db);
#define SQL_GetAutocommit(db) L_SQL_GetAutocommit(db)
int L_SQL_TxnState(sqlite3 *db, const char *zSchema);
#define SQL_TxnState(db,zSchema) L_SQL_TxnState(db,zSchema)
sqlite3 * L_SQL_DbHandle(sqlite3_stmt *pStmt);
#define SQL_DbHandle(pStmt) L_SQL_DbHandle(pStmt)
const char * L_SQL_DbFilename(sqlite3 *db, const char *zDbName);
#define SQL_DbFilename(db,zDbName) L_SQL_DbFilename(db,zDbName)
int L_SQL_DbReadonly(sqlite3 *db, const char *zDbName);
#define SQL_DbReadonly(db,zDbName) L_SQL_DbReadonly(db,zDbName)
sqlite3_stmt * L_SQL_NextStmt(sqlite3 *db, sqlite3_stmt *pStmt);
#define SQL_NextStmt(db,pStmt) L_SQL_NextStmt(db,pStmt)
void * L_SQL_CommitHook(sqlite3 *db, APTR xCallback, void *pArg);
#define SQL_CommitHook(db,xCallback,pArg) L_SQL_CommitHook(db,xCallback,pArg)
void * L_SQL_RollbackHook(sqlite3 *db, APTR xCallback, void *pArg);
#define SQL_RollbackHook(db,xCallback,pArg) L_SQL_RollbackHook(db,xCallback,pArg)
void * L_SQL_UpdateHook(sqlite3 *db, APTR xCallback, void *pArg);
#define SQL_UpdateHook(db,xCallback,pArg) L_SQL_UpdateHook(db,xCallback,pArg)
int L_SQL_TraceV2(sqlite3 *db, unsigned int uMask, APTR xCallback, void *pCtx);
#define SQL_TraceV2(db,uMask,xCallback,pCtx) L_SQL_TraceV2(db,uMask,xCallback,pCtx)
int L_SQL_DbReleaseMemory(sqlite3 *db);
#define SQL_DbReleaseMemory(db) L_SQL_DbReleaseMemory(db)
int L_SQL_DbCacheflush(sqlite3 *db);
#define SQL_DbCacheflush(db) L_SQL_DbCacheflush(db)
int L_SQL_FileControl(sqlite3 *db, const char *zDbName, int op, void *pArg);
#define SQL_FileControl(db,zDbName,op,pArg) L_SQL_FileControl(db,zDbName,op,pArg)
void * L_SQL_WalHook(sqlite3 *db, APTR xCallback, void *pArg);
#define SQL_WalHook(db,xCallback,pArg) L_SQL_WalHook(db,xCallback,pArg)
int L_SQL_WalCheckpointV2(sqlite3 *db, const char *zDb, int eMode, int *pnLog, int *pnCkpt);
#define SQL_WalCheckpointV2(db,zDb,eMode,pnLog,pnCkpt) L_SQL_WalCheckpointV2(db,zDb,eMode,pnLog,pnCkpt)
sqlite3_str * L_SQL_StrNew(sqlite3 *db);
#define SQL_StrNew(db) L_SQL_StrNew(db)
char * L_SQL_StrFinish(sqlite3_str *p);
#define SQL_StrFinish(p) L_SQL_StrFinish(p)
void L_SQL_StrVappendf(sqlite3_str *p, const char *zFormat, va_list ap);
#define SQL_StrVappendf(p,zFormat,ap) L_SQL_StrVappendf(p,zFormat,ap)
void L_SQL_StrAppend(sqlite3_str *p, const char *zIn, int N);
#define SQL_StrAppend(p,zIn,N) L_SQL_StrAppend(p,zIn,N)
void L_SQL_StrAppendall(sqlite3_str *p, const char *zIn);
#define SQL_StrAppendall(p,zIn) L_SQL_StrAppendall(p,zIn)
void L_SQL_StrAppendchar(sqlite3_str *p, int N, char C);
#define SQL_StrAppendchar(p,N,C) L_SQL_StrAppendchar(p,N,C)
void L_SQL_StrReset(sqlite3_str *p);
#define SQL_StrReset(p) L_SQL_StrReset(p)
int L_SQL_StrErrcode(sqlite3_str *p);
#define SQL_StrErrcode(p) L_SQL_StrErrcode(p)
int L_SQL_StrLength(sqlite3_str *p);
#define SQL_StrLength(p) L_SQL_StrLength(p)
char * L_SQL_StrValue(sqlite3_str *p);
#define SQL_StrValue(p) L_SQL_StrValue(p)
char * L_SQL_Vmprintf(const char *zFormat, va_list ap);
#define SQL_Vmprintf(zFormat,ap) L_SQL_Vmprintf(zFormat,ap)
char * L_SQL_Vsnprintf(int n, char *zBuf, const char *zFormat, va_list ap);
#define SQL_Vsnprintf(n,zBuf,zFormat,ap) L_SQL_Vsnprintf(n,zBuf,zFormat,ap)
int L_SQL_Stricmp(const char *a, const char *b);
#define SQL_Stricmp(a,b) L_SQL_Stricmp(a,b)
int L_SQL_Strnicmp(const char *a, const char *b, int n);
#define SQL_Strnicmp(a,b,n) L_SQL_Strnicmp(a,b,n)
int L_SQL_Strglob(const char *zGlob, const char *zStr);
#define SQL_Strglob(zGlob,zStr) L_SQL_Strglob(zGlob,zStr)
int L_SQL_Strlike(const char *zGlob, const char *zStr, unsigned int cEsc);
#define SQL_Strlike(zGlob,zStr,cEsc) L_SQL_Strlike(zGlob,zStr,cEsc)
int L_SQL_Sleep(int ms);
#define SQL_Sleep(ms) L_SQL_Sleep(ms)
sqlite3_vfs * L_SQL_VfsFind(const char *zVfsName);
#define SQL_VfsFind(zVfsName) L_SQL_VfsFind(zVfsName)

#endif
