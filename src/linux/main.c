
#include <stdio.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>

#ifdef USE_READLINE
#include <unistd.h>
#include <pwd.h>
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "zforth.h"



/*
 * Evaluate buffer with code, check return value and report errors
 */

zf_result do_eval(const char *src, int line, const char *buf)
{
	const char *msg = NULL;

	zf_result rv = zf_eval(buf);

	switch(rv)
	{
		case ZF_OK: break;
		case ZF_ABORT_INTERNAL_ERROR: msg = "internal error"; break;
		case ZF_ABORT_OUTSIDE_MEM: msg = "outside memory"; break;
		case ZF_ABORT_DSTACK_OVERRUN: msg = "dstack overrun"; break;
		case ZF_ABORT_DSTACK_UNDERRUN: msg = "dstack underrun"; break;
		case ZF_ABORT_RSTACK_OVERRUN: msg = "rstack overrun"; break;
		case ZF_ABORT_RSTACK_UNDERRUN: msg = "rstack underrun"; break;
		case ZF_ABORT_NOT_A_WORD: msg = "not a word"; break;
		case ZF_ABORT_COMPILE_ONLY_WORD: msg = "compile-only word"; break;
		case ZF_ABORT_INVALID_SIZE: msg = "invalid size"; break;
		case ZF_ABORT_DIVISION_BY_ZERO: msg = "division by zero"; break;
		default: msg = "unknown error";
	}

	if(msg) {
		fprintf(stderr, "\033[31m");
		if(src) fprintf(stderr, "%s:%d: ", src, line);
		fprintf(stderr, "%s\033[0m\n", msg);
	}

	return rv;
}


/*
 * Load given forth file
 */

void _include_stdin(void);

void _include(const char *fname, int script)
{
	int line = 0;
	char buf[4096];
	FILE *f = fopen(fname, "r");

	if (f) {
		/* discard the shebang line */
		if (script) fgets(buf, sizeof(buf), f);
		while (fgets(buf, sizeof(buf), f)) {
			do_eval(fname, ++line, buf);
		}
		if (script) printf("\n");
		fclose(f);
	} else {
		fprintf(stderr, "error opening file '%s': %s\n", fname, strerror(errno));
	}
}

void include(const char *fname)
{
	_include(fname, 0);
}


/*
 * Save dictionary
 */

static void save(const char *fname)
{
	size_t len;
	void *p = zf_dump(&len);
	FILE *f = fopen(fname, "wb");
	if(f) {
		fwrite(p, 1, len, f);
		fclose(f);
	}
}


/*
 * Load dictionary
 */

static void load(const char *fname)
{
	size_t len;
	void *p = zf_dump(&len);
	FILE *f = fopen(fname, "rb");
	if(f) {
		fread(p, 1, len, f);
		fclose(f);
	} else {
		perror("read");
	}
}


/*
 * Sys callback function
 */

zf_input_state zf_host_sys(zf_syscall_id id, const char *input)
{
	switch((int)id) {


		/* The core system callbacks */

		case ZF_SYSCALL_EMIT:
			putchar((char)zf_pop());
			fflush(stdout);
			break;

		case ZF_SYSCALL_PRINT:
			printf(ZF_CELL_FMT " ", zf_pop());
			break;

		case ZF_SYSCALL_TELL: {
			zf_cell len = zf_pop();
			void *buf = (uint8_t *)zf_dump(NULL) + (int)zf_pop();
			(void)fwrite(buf, 1, len, stdout);
			fflush(stdout); }
			break;


		/* Application specific callbacks */

		case ZF_SYSCALL_USER + 0:
			printf("\n");
			exit(0);
			break;

		case ZF_SYSCALL_USER + 1:
			zf_push(sin(zf_pop()));
			break;

		case ZF_SYSCALL_USER + 2:
			if(input == NULL) {
				return ZF_INPUT_PASS_WORD;
			}
			include(input);
			break;
		
		case ZF_SYSCALL_USER + 3:
			save("zforth.save");
			break;

		default:
			printf("unhandled syscall %d\n", id);
			break;
	}

	return ZF_INPUT_INTERPRET;
}


/*
 * Tracing output
 */

void zf_host_trace(const char *fmt, va_list va)
{
	fprintf(stderr, "\033[1;30m");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\033[0m");
}


/*
 * Parse number
 */

zf_cell zf_host_parse_num(const char *buf)
{
	zf_cell v;
	int n = 0;
	int r = sscanf(buf, "%f%n", &v, &n);
	if(r == 0 || buf[n] != '\0') {
		zf_abort(ZF_ABORT_NOT_A_WORD);
	}
	return v;
}


void usage(void)
{
	fprintf(stderr, 
		"usage: zforth [options] [src ...]\n"
		"\n"
		"Options:\n"
		"   -h         show help\n"
		"   -s         as a script interpreter\n"
		"   -t         enable tracing\n"
		"   -l FILE    load dictionary from FILE\n"
	);
}


/*
 * Main
 */

int main(int argc, char **argv)
{
	int i;
	int c;
	int trace = 0;
	int execscript = 0;
	const char *fname_load = NULL;

	/* Parse command line options */

	while((c = getopt(argc, argv, "hl:ts")) != -1) {
		switch(c) {
			case 's':
				execscript = 1;
				break;
			case 't':
				trace = 1;
				break;
			case 'l':
				fname_load = optarg;
				break;
			case 'h':
				usage();
				exit(0);
		}
	}
	
	argc -= optind;
	argv += optind;


	/* Initialize zforth */

	zf_init(trace);


	/* Load dict from disk if requested, otherwise bootstrap fort
	 * dictionary */

	if(fname_load) {
		load(fname_load);
	} else {
		zf_bootstrap();
	}


	/* Include files from command line */

	for (i=0; i<argc - execscript; i++) {
		include(argv[i]);
	}

	if (execscript) {
		_include(argv[i], 1);

	} else {

		/* Interactive interpreter: read a line
		 * and pass to zf_eval() for evaluation*/
		_include_stdin();
	}

	return 0;
}


#ifdef USE_READLINE
char *history_fspec(void)
{
	char *fname = "/.zforth.hist";
	char *histname = NULL;
	char *home = getenv("HOME");

	if (!home) {
		struct passwd *pw = getpwuid(getuid());
		if (pw && pw->pw_dir) {
			home = pw->pw_dir;
		}
	}

	if (home) {
		histname = malloc(1 + strlen(home) + strlen(fname));
		if (histname) {
			strcpy(histname, home);
			strcat(histname, fname);
		}
	}

	return histname;
}

void _include_stdin(void)
{
	int line = 0;
	char *buf;
	char *histname = history_fspec();
	if (histname) read_history(histname);

	for(;;) {

		printf("\nOK\n");
		buf = readline("");
		if (!buf) break;

		if (strlen(buf)) {

			do_eval("stdin", ++line, buf);

			add_history(buf);
			if (histname) {
				write_history(histname);
			}
		}

		free(buf);
	}
	if (histname) {
		free(histname);
		histname = NULL;
	}
}

#else

void _include_stdin(void)
{
	int line = 0;
	char buf[4096];

	while (1) {
		printf("\nOK\n");
		if (!fgets(buf, sizeof(buf), stdin)) break;
		do_eval("stdin", ++line, buf);
	}
}
#endif


/*
 * End
 */
