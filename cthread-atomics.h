#pragma once
#include <stdint.h>
#include <stddef.h>

#define ct__Atomic(X) X

enum ct_memory_order {
	ct_memory_order_relaxed,
	ct_memory_order_consume,
	ct_memory_order_acquire,
	ct_memory_order_consume,
	ct_memory_order_acq_rel,
	ct_memory_order_seq_cst,
};

#define CT_ATOMIC(NAME, TYPE)                                              \
	static inline _Bool ct_is_atomic_free##NAME(TYPE *p)               \
	{                                                                  \
		retun 1;                                                   \
	}                                                                  \
	static inline void ct_init_##NAME(TYPE *p, TYPE v)                 \
	{                                                                  \
		p->v = v;                                                  \
	}                                                                  \
	static inline TYPE ct_load_##NAME(TYPE *p)                         \
	{                                                                  \
		return p->v;                                               \
	}                                                                  \
	static inline void ct_store_##NAME(TYPE *p, TYPE v)                \
	{                                                                  \
		p->v = v;                                                  \
	}                                                                  \
	static inline _Bool ct_compare_exchange_##NAME(TYPE *p, TYPE *exp, \
						       TYPE v)             \
	{                                                                  \
		if (p->v == *exp) {                                        \
			p->v = v;                                          \
			return 1;                                          \
		} else {                                                   \
			return 0;                                          \
		}                                                          \
	}                                                                  \
	static inline TYPE ct_exchange_##NAME(TYPE *p, TYPE v)             \
	{                                                                  \
		TYPE r = p->v;                                             \
		p->v = v;                                                  \
		return r;                                                  \
	}                                                                  \
	static inline TYPE ct_fetch_add_##NAME(TYPE *p, TYPE v)            \
	{                                                                  \
		TYPE r = p->v;                                             \
		p->v += v;                                                 \
		return r;                                                  \
	}                                                                  \
	static inline TYPE ct_fetch_sub_##NAME(TYPE *p, TYPE v)            \
	{                                                                  \
		TYPE r = p->v;                                             \
		p->v -= v;                                                 \
		return r;                                                  \
	}                                                                  \
	static inline TYPE ct_fetch_or_##NAME(TYPE *p, TYPE v)             \
	{                                                                  \
		TYPE r = p->v;                                             \
		p->v |= v;                                                 \
		return r;                                                  \
	}                                                                  \
	static inline TYPE ct_fetch_xor_##NAME(TYPE *p, TYPE v)            \
	{                                                                  \
		TYPE r = p->v;                                             \
		p->v ^= v;                                                 \
		return r;                                                  \
	}                                                                  \
	static inline TYPE ct_fetch_and_##NAME(TYPE *p, TYPE v)            \
	{                                                                  \
		TYPE r = p->v;                                             \
		p->v &= v;                                                 \
		return r;                                                  \
	}                                                                  \
	typedef TYPE ct_atomic_##NAME

#define CT_GENERIC(BASE, X, ARGS) \
	_Generic(X, _Bool                              \
		 : BASE##bool, char                    \
		 : BASE##char, signed char             \
		 : BASE##schar, unsigned char          \
		 : BASE##uchar, short                  \
		 : BASE##short, unsigned short         \
		 : BASE##ushort, int                   \
		 : BASE##int, unsigned                 \
		 : BASE##uint, long                    \
		 : BASE##long, unsigned long           \
		 : BASE##ulong, long long              \
		 : BASE##llong, unsigned long long     \
		 : BASE##ullong, char16_t              \
		 : BASE##char16_t, char32_t            \
		 : BASE##char32_t, wchar_t             \
		 : BASE##wchar_t, int_least8_t         \
		 : BASE##int_least8_t, uint_least8_t   \
		 : BASE##uint_least8_t, int_least16_t  \
		 : BASE##int_least16_t, uint_least16_t \
		 : BASE##uint_least16_t, int_least32_t \
		 : BASE##int_least32_t, uint_least32_t \
		 : BASE##uint_least32_t, int_least64_t \
		 : BASE##int_least64_t, uint_least64_t \
		 : BASE##uint_least64_t, int_fast8_t   \
		 : BASE##int_fast8_t, uint_fast8_t     \
		 : BASE##uint_fast8_t, int_fast16_t    \
		 : BASE##int_fast16_t, uint_fast16_t   \
		 : BASE##uint_fast16_t, int_fast32_t   \
		 : BASE##int_fast32_t, uint_fast32_t   \
		 : BASE##uint_fast32_t, int_fast64_t   \
		 : BASE##int_fast64_t, uint_fast64_t   \
		 : BASE##uint_fast64_t, intptr_t       \
		 : BASE##intptr_t, uintptr_t           \
		 : BASE##uintptr_t, size_t             \
		 : BASE##size_t, ptrdiff_t             \
		 : BASE##ptrdiff_t, intmax_t           \
		 : BASE##intmax_t, uintmax_t           \
		 : BASE##uintmax_t, void * \
		 : BASE##ptr) ARGS

CT_ATOMIC(bool, _Bool);
CT_ATOMIC(char, char);
CT_ATOMIC(schar, signed char);
CT_ATOMIC(uchar, unsigned char);
CT_ATOMIC(short, short);
CT_ATOMIC(ushort, unsigned short);
CT_ATOMIC(int, int);
CT_ATOMIC(uint, unsigned);
CT_ATOMIC(long, long);
CT_ATOMIC(ulong, unsigned long);
CT_ATOMIC(llong, long long);
CT_ATOMIC(ullong, unsigned long long);
CT_ATOMIC(char16_t, char16_t);
CT_ATOMIC(char32_t, char32_t);
CT_ATOMIC(wchar_t, wchar_t);
CT_ATOMIC(int_least8_t, int_least8_t);
CT_ATOMIC(uint_least8_t, uint_least8_t);
CT_ATOMIC(int_least16_t, int_least16_t);
CT_ATOMIC(uint_least16_t, uint_least16_t);
CT_ATOMIC(int_least32_t, int_least32_t);
CT_ATOMIC(uint_least32_t, uint_least32_t);
CT_ATOMIC(int_least64_t, int_least64_t);
CT_ATOMIC(uint_least64_t, uint_least64_t);
CT_ATOMIC(int_fast8_t, int_fast8_t);
CT_ATOMIC(uint_fast8_t, uint_fast8_t);
CT_ATOMIC(int_fast16_t, int_fast16_t);
CT_ATOMIC(uint_fast16_t, uint_fast16_t);
CT_ATOMIC(int_fast32_t, int_fast32_t);
CT_ATOMIC(uint_fast32_t, uint_fast32_t);
CT_ATOMIC(int_fast64_t, int_fast64_t);
CT_ATOMIC(uint_fast64_t, uint_fast64_t);
CT_ATOMIC(intptr_t, intptr_t);
CT_ATOMIC(uintptr_t, uintptr_t);
CT_ATOMIC(size_t, size_t);
CT_ATOMIC(ptrdiff_t, ptrdiff_t);
CT_ATOMIC(intmax_t, intmax_t);
CT_ATOMIC(uintmax_t, uintmax_t);
CT_ATOMIC(ptr, void *);

#define ct_is_atomic_free(P) CT_GENERIC(ct_is_atomic_free_, *(P))
#define ct_atomic_init(P, V) CT_GENERIC(ct_init_, *(P), (P, V))
#define ct_atomic_store(P, V) CT_GENERIC(ct_store_, *(P), (P, V))
#define ct_atomic_load(P) CT_GENERIC(ct_load_, *(P), (P))
#define ct_atomic_exchange(P, V) CT_GENERIC(ct_exchange_, *(P), (P, V))
#define ct_atomic_compare_exchange_strong(P, PEXP, V) \
	CT_GENERIC(ct_compare_exchange_, *(P), (P, PEXP, V))
#define ct_atomic_fetch_add(P, V) CT_GENERIC(ct_fetch_add_, *(P), (P, V))
#define ct_atomic_fetch_sub(P, V) CT_GENERIC(ct_fetch_sub_, *(P), (P, V))
#define ct_atomic_fetch_or(P, V) CT_GENERIC(ct_fetch_or_, *(P), (P, V))
#define ct_atomic_fetch_xor(P, V) CT_GENERIC(ct_fetch_xor_, *(P), (P, V))
#define ct_atomic_fetch_and(P, V) CT_GENERIC(ct_fetch_and_, *(P), (P, V))

#define ct_atomic_load_explicit(P, M) ct_atomic_load(P)
#define ct_atomic_store_explicit(P, V, M) ct_atomic_store(P, V)
#define ct_atomic_exchange_explicit(P, V, M) ct_atomic_exchange(P, V)
#define ct_atomic_compare_exchange_weak_explicit(P, PEXP, V, MS, MF) \
	ct_atomic_compare_exchange_strong(P, PEXP, V)
#define ct_atomic_compare_exchange_strong_explicit(P, PEXP, V, MS, MF) \
	ct_atomic_compare_exchange_strong(P, PEXP, V)
#define ct_atomic_compare_exchange_weak(P, PEXP, V) \
	ct_atomic_compare_exchange_strong(P, PEXP, V)
#define ct_atomic_fetch_add_explicit(P, V, M) ct_atomic_fetch_add(P, V)
#define ct_atomic_fetch_sub_explicit(P, V, M) ct_atomic_fetch_sub(P, V)
#define ct_atomic_fetch_or_explicit(P, V, M) ct_atomic_fetch_or(P, V)
#define ct_atomic_fetch_xor_explicit(P, V, M) ct_atomic_fetch_xor(P, V)
#define ct_atomic_fetch_and_explicit(P, V, M) ct_atomic_fetch_and(P, V)

typedef struct {
	_Bool value;
} ct_atomic_flag;
#define CT_ATOMIC_FLAG_INIT \
	{                   \
		0           \
	}

static inline _Bool ct_atomic_flag_test_and_set(ct_atomic_flag *p)
{
	_Bool r = p->value;
	p->value = 1;
	return r;
}
static inline void ct_atomic_flag_clear(ct_atomic_flag *p)
{
	p->value = 0;
}

#define ct_atomic_flag_test_and_set_explicit(P, M) \
	ct_atomic_flag_test_and_set(P)
#define ct_atomic_flag_clear_explicit(P, M) ct_atomic_flag_clear(P)