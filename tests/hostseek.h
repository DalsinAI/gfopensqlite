/* gfopensqlite: AmigaDOS's seek on the host (tests/host-seektest.sh).
 * Seek() on a real AmigaDOS disk refuses a position past the end of the
 * file; this lseek() does the same, so the amalgamation meets on the host
 * what it meets on Work:. Each refusal is counted.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef GFOPENSQLITE_HOSTSEEK_H
#define GFOPENSQLITE_HOSTSEEK_H
#include <sys/types.h>
#include <unistd.h>
off_t host_amiga_lseek(int fd, off_t off, int whence);
extern long host_amiga_refused;
#define lseek host_amiga_lseek
#endif
