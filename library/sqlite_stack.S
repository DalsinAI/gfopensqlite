| sqlite.library: a call on a stack of its own, and the FPU's control
| register (the only FPU instructions in the library outside fpuauto; they
| run only when exec says there is an FPU).
| MIT, Copyright (c) 2026 AmiCradle Team.

	.text
	.even

| LONG sql_onstack(LONG (*fn)(APTR), APTR arg, struct StackSwapStruct *sss)
| fn(arg) on the stack sss describes (exec StackSwap, -732), then back.
	.globl	_sql_onstack
_sql_onstack:
	movem.l	d2-d3/a2-a3/a6,-(sp)
	move.l	24(sp),a2		| fn
	move.l	28(sp),d2		| arg
	move.l	32(sp),a3		| sss
	move.l	4.w,a6
	move.l	a3,a0
	jsr	-732(a6)		| StackSwap: now on the new stack
	move.l	d2,-(sp)
	jsr	(a2)
	addq.l	#4,sp
	move.l	d0,d3
	move.l	4.w,a6
	move.l	a3,a0
	jsr	-732(a6)		| StackSwap: back
	move.l	d3,d0
	movem.l	(sp)+,d2-d3/a2-a3/a6
	rts

| ULONG sql_fpcr_get(void); void sql_fpcr_set(ULONG)
	.globl	_sql_fpcr_get
_sql_fpcr_get:
	fmove.l	fpcr,d0
	rts

	.globl	_sql_fpcr_set
_sql_fpcr_set:
	move.l	4(sp),d0
	fmove.l	d0,fpcr
	rts
