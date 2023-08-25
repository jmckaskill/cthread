
#ifdef _WIN32
#include <windows.h>
void create_tls_key(volatile long *ponce, tls_key_t *pkey)
{
	if (InterlockedCompareExchange(ponce, 1, 0) == 0) {
		*pkey = TlsAlloc();
		if (*pkey == TLS_OUT_OF_INDEXES) {
			abort();
		}
		InterlockedExchange(ponce, 2);
	} else {
		while (*ponce == 1) {
			Sleep(0);
		}
	}
}

#else
#include <stdatomic.h>
void create_tls_key(volatile long *ponce, tls_key_t *pkey)
{
	long have = 0;
	if (atomic_compare_exchange_strong_explicit(ponce, &have, 1,
						    memory_order_relaxed,
						    memory_order_relaxed)) {
		pthread_key_t key;
		if (pthread_key_create(&key, NULL)) {
			abort();
		}
		*pkey = (tls_key_t)key;
		atomic_store_explicit(ponce, 2, memory_order_release);
	} else {
		while (atomic_load_explicit(ponce, memory_order_acquire) == 1) {
			usleep(0);
		}
	}
}
#endif