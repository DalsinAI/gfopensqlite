/* sqlite.library's AmigaOS layer (sqlite_amiga.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef SQLITE_AMIGA_H
#define SQLITE_AMIGA_H

void sql_amiga_init(void);      /* once, when the library is loaded */
int sql_amiga_start(void);      /* before sqlite3_initialize */
void sql_amiga_end(void);       /* after sqlite3_shutdown */

#endif
