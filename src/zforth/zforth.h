#ifndef zforth_h
#define zforth_h

#include "zfconf.h"

/* Abort reasons */

typedef enum {
	ZF_OK, // 0
	ZF_ABORT_INTERNAL_ERROR, // 1
	ZF_ABORT_OUTSIDE_MEM, // 2
	ZF_ABORT_DSTACK_UNDERRUN, // 3
	ZF_ABORT_DSTACK_OVERRUN, // 4
	ZF_ABORT_RSTACK_UNDERRUN, // 5
	ZF_ABORT_RSTACK_OVERRUN, // 6
	ZF_ABORT_NOT_A_WORD, // 7
	ZF_ABORT_COMPILE_ONLY_WORD, // 8
	ZF_ABORT_INVALID_SIZE, // 9
	ZF_ABORT_DIVISION_BY_ZERO, // A
	ZF_ABORT_UNKNOWN_SYS, // B
	ZF_ABORT_UNKNOWN, // C
} zf_result;

typedef enum {
	ZF_MEM_SIZE_VAR,
	ZF_MEM_SIZE_CELL,
	ZF_MEM_SIZE_U8,
	ZF_MEM_SIZE_U16,
	ZF_MEM_SIZE_U32,
	ZF_MEM_SIZE_S8,
	ZF_MEM_SIZE_S16,
	ZF_MEM_SIZE_S32
} zf_mem_size;

typedef enum {
	ZF_INPUT_INTERPRET,
	ZF_INPUT_PASS_CHAR,
	ZF_INPUT_PASS_WORD
} zf_input_state;

typedef enum {
	ZF_SYSCALL_EMIT,
	ZF_SYSCALL_PRINT,
	ZF_SYSCALL_TELL,
	ZF_SYSCALL_USER = 128
} zf_syscall_id;


/* ZForth API functions */


void zf_init();
void zf_bootstrap(void);
void *zf_dump(size_t *len);
zf_result zf_eval(const char *buf);
void zf_abort(zf_result reason);

void zf_push(zf_cell v);
zf_cell zf_pop(void);
zf_cell zf_pick(zf_addr n);
zf_cell zf_sc();

/* Host provides these functions */

zf_input_state zf_host_sys(zf_syscall_id id, const char *last_word);
void zf_host_trace(const char *fmt, va_list va);
zf_cell zf_host_parse_num(const char *buf);

#endif
