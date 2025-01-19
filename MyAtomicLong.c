#include "MyAtomicLong.h"

int MyAtomicLong_init(MyAtomicLong_t *ctx)
{
	ctx->hMutex = CreateMutex( 
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	ctx->value = 0;

	if(ctx->hMutex == INVALID_HANDLE_VALUE) return -1;

	return 1;
}

int MyAtomicLong_destroy(MyAtomicLong_t *ctx)
{
	CloseHandle(ctx->hMutex);

	return 1;
}

long MyAtomicLong_getAndplus(MyAtomicLong_t *ctx, long value)
{
	DWORD dwRst;

	long ret_value;

	dwRst = WaitForSingleObject( 
		ctx->hMutex,    // handle to mutex
		INFINITE);      // no time-out interval

	ret_value = ctx->value;
	ctx->value += value;

	ReleaseMutex(ctx->hMutex);

	return ret_value;
}

long MyAtomicLong_getAndminus(MyAtomicLong_t *ctx, long value)
{
	DWORD dwRst;

	long ret_value;

	dwRst = WaitForSingleObject( 
		ctx->hMutex,    // handle to mutex
		INFINITE);      // no time-out interval

	ret_value = ctx->value;
	ctx->value -= value;

	ReleaseMutex(ctx->hMutex);

	return ret_value;
}

long MyAtomicLong_getAndset(MyAtomicLong_t *ctx, long value)
{
	DWORD dwRst;

	long ret_value;

	dwRst = WaitForSingleObject( 
		ctx->hMutex,    // handle to mutex
		INFINITE);      // no time-out interval

	ret_value = ctx->value;
	ctx->value = value;

	ReleaseMutex(ctx->hMutex);

	return ret_value;
}

long MyAtomicLong_get(MyAtomicLong_t *ctx)
{
	DWORD dwRst;

	long ret_value;

	dwRst = WaitForSingleObject( 
		ctx->hMutex,    // handle to mutex
		INFINITE);      // no time-out interval

	ret_value = ctx->value;

	ReleaseMutex(ctx->hMutex);

	return ret_value;
}