#ifndef __MYATOMICLONG_H__
#define __MYATOMICLONG_H__

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _tag_MyAtomicLong {
	HANDLE hMutex;
	volatile long value;
} MyAtomicLong_t;

int MyAtomicLong_init(MyAtomicLong_t *ctx);
int MyAtomicLong_destroy(MyAtomicLong_t *ctx);
long MyAtomicLong_getAndplus(MyAtomicLong_t *ctx, long value);
long MyAtomicLong_getAndminus(MyAtomicLong_t *ctx, long value);
long MyAtomicLong_getAndset(MyAtomicLong_t *ctx, long value);
long MyAtomicLong_get(MyAtomicLong_t *ctx);

#ifdef __cplusplus
}
#endif

#endif