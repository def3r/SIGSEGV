#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX(a, b) (a > b) ? a : b
#define MIN(a, b) (a < b) ? a : b

__attribute__((__noreturn__)) void abort(void);

#ifdef __cplusplus
}
#endif

#endif
