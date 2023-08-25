#pragma once

typedef unsigned tls_key_t;

tls_key_t create_tls_key(volatile long *ponce);

#ifdef _WIN32
#include <windows.h>
void *tls_get(tls_key_t key)
{
	return TlsGetValue(key);
}
void tls_set(tls_key_t key, void *data)
{
	TlsSetValue(key, value);
}
#else
#include <pthread.h>
void *tls_get(tls_key_t key)
{
	return pthread_getspecific(key);
}
void tls_set(tls_key_t key, void *data)
{
	pthread_setspecific(key, data);
}
#endif