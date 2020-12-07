/**
 * cd_hash.h - Hashtable implementation
 *
 * Part of the libcd - bringing you support for C programs with queue processors, from Data And Signal's Piotr Gregor
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */


#ifndef CD_HASHTABLE_H
#define CD_HASHTABLE_H


#include "cd_list.h"


#include <stdint.h>


/* TODO handle n < 0 case */
#define cd_ilog2(n)	            \
(			                    \
	(                           \
	(n) & (1ULL << 63) ? 63 :	\
	(n) & (1ULL << 62) ? 62 :	\
	(n) & (1ULL << 61) ? 61 :	\
	(n) & (1ULL << 60) ? 60 :	\
	(n) & (1ULL << 59) ? 59 :	\
	(n) & (1ULL << 58) ? 58 :	\
	(n) & (1ULL << 57) ? 57 :	\
	(n) & (1ULL << 56) ? 56 :	\
	(n) & (1ULL << 55) ? 55 :	\
	(n) & (1ULL << 54) ? 54 :	\
	(n) & (1ULL << 53) ? 53 :	\
	(n) & (1ULL << 52) ? 52 :	\
	(n) & (1ULL << 51) ? 51 :	\
	(n) & (1ULL << 50) ? 50 :	\
	(n) & (1ULL << 49) ? 49 :	\
	(n) & (1ULL << 48) ? 48 :	\
	(n) & (1ULL << 47) ? 47 :	\
	(n) & (1ULL << 46) ? 46 :	\
	(n) & (1ULL << 45) ? 45 :	\
	(n) & (1ULL << 44) ? 44 :	\
	(n) & (1ULL << 43) ? 43 :	\
	(n) & (1ULL << 42) ? 42 :	\
	(n) & (1ULL << 41) ? 41 :	\
	(n) & (1ULL << 40) ? 40 :	\
	(n) & (1ULL << 39) ? 39 :	\
	(n) & (1ULL << 38) ? 38 :	\
	(n) & (1ULL << 37) ? 37 :	\
	(n) & (1ULL << 36) ? 36 :	\
	(n) & (1ULL << 35) ? 35 :	\
	(n) & (1ULL << 34) ? 34 :	\
	(n) & (1ULL << 33) ? 33 :	\
	(n) & (1ULL << 32) ? 32 :	\
	(n) & (1ULL << 31) ? 31 :	\
	(n) & (1ULL << 30) ? 30 :	\
	(n) & (1ULL << 29) ? 29 :	\
	(n) & (1ULL << 28) ? 28 :	\
	(n) & (1ULL << 27) ? 27 :	\
	(n) & (1ULL << 26) ? 26 :	\
	(n) & (1ULL << 25) ? 25 :	\
	(n) & (1ULL << 24) ? 24 :	\
	(n) & (1ULL << 23) ? 23 :	\
	(n) & (1ULL << 22) ? 22 :	\
	(n) & (1ULL << 21) ? 21 :	\
	(n) & (1ULL << 20) ? 20 :	\
	(n) & (1ULL << 19) ? 19 :	\
	(n) & (1ULL << 18) ? 18 :	\
	(n) & (1ULL << 17) ? 17 :	\
	(n) & (1ULL << 16) ? 16 :	\
	(n) & (1ULL << 15) ? 15 :	\
	(n) & (1ULL << 14) ? 14 :	\
	(n) & (1ULL << 13) ? 13 :	\
	(n) & (1ULL << 12) ? 12 :	\
	(n) & (1ULL << 11) ? 11 :	\
	(n) & (1ULL << 10) ? 10 :	\
	(n) & (1ULL <<  9) ?  9 :	\
	(n) & (1ULL <<  8) ?  8 :	\
	(n) & (1ULL <<  7) ?  7 :	\
	(n) & (1ULL <<  6) ?  6 :	\
	(n) & (1ULL <<  5) ?  5 :	\
	(n) & (1ULL <<  4) ?  4 :	\
	(n) & (1ULL <<  3) ?  3 :	\
	(n) & (1ULL <<  2) ?  2 :	\
	(n) & (1ULL <<  1) ?  1 :	\
	0 				\
	)				\
 )

/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL

/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define GOLDEN_RATIO_PRIME_64 0x9e37fffffffc0001UL

#ifndef CD_BITS_PER_LONG
	#if ULONG_MAX > UINT_MAX
		#define CD_BITS_PER_LONG 64
	#else
		#define CD_BITS_PER_LONG 32
	#endif
	#if CD_BITS_PER_LONG == 32
		#define CD_GOLDEN_RATIO_PRIME GOLDEN_RATIO_PRIME_32
		#define cd_hash_long(val, bits) cd_hash_32(val, bits)
	#elif CD_BITS_PER_LONG == 64
		#define cd_hash_long(val, bits) cd_hash_64(val, bits)
		#define CD_GOLDEN_RATIO_PRIME GOLDEN_RATIO_PRIME_64
	#else
		#error Wordsize not 32 or 64 CD_BITS_PER_LONG
	#endif
#endif

static uint64_t
cd_hash_64(uint64_t val, unsigned int bits) {
	uint64_t hash = val;
	uint64_t n = hash;
	n <<= 18;
	hash -= n;
	n <<= 33;
	hash -= n;
	n <<= 3;
	hash += n;
	n <<= 3;
	hash -= n;
	n <<= 4;
	hash += n;
	n <<= 2;
	hash += n;

	/* High bits are more random, so use them. */
	return hash >> (64 - bits);
}

static uint32_t
cd_hash_32(uint32_t val, unsigned int bits) {
	/* On some cpus multiply is faster, on others gcc will do shifts */
	uint32_t hash = val * GOLDEN_RATIO_PRIME_32;

	/* High bits are more random, so use them. */
	return hash >> (32 - bits);
}

static unsigned long
cd_hash_ptr(const void *ptr, unsigned int bits) {
	return cd_hash_long((unsigned long)ptr, bits);
}

static uint32_t
cd_hash32_ptr(const void *ptr) {
	unsigned long val = (unsigned long) ptr;

#if CD_BITS_PER_LONG == 64
	val ^= (val >> 32);
#endif
	return (uint32_t) val;
}

#define CD_ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/* not supported by ISO C
#define CD_DEFINE_AND_INIT_HASHTABLE(name, bits)				\
	struct cd_hlist_head name[1 << (bits)] =			\
		{ [0 ... ((1 << (bits)) - 1)] = CD_HLIST_HEAD_INIT }
*/
#define CD_DEFINE_HASHTABLE(name, bits) struct cd_hlist_head name[1 << (bits)]

#define CD_DECLARE_HASHTABLE(name, bits)	\
	struct cd_hlist_head name[1 << (bits)]

#define CD_HASH_SIZE(name) (CD_ARRAY_SIZE(name))
#define CD_HASH_BITS(name) cd_ilog2(CD_HASH_SIZE(name))

/* Use cd_hash_32 when possible to allow for fast
 * 32bit hashing in 64bit kernels. */
#define cd_hash_min(val, bits)							\
	(sizeof(val) <= 4 ? cd_hash_32(val, bits) \
	 	: cd_hash_long(val, bits))

static void
__cd_hash_init(struct cd_hlist_head *ht, size_t sz) {
	size_t i;
	for (i = 0; i < sz; i++)
		CD_INIT_HLIST_HEAD(&ht[i]);
}

/* @brief	Initialize a hashtable.
 * @details	Calculates the size of the hashtable from the given parameter, otherwise
 * same as cd_hash_init_size.
 * This has to be a macro since HASH_BITS() will not work on pointers since
 * it calculates the size during preprocessing.
 * @cd_hashtable: hashtable to be initialized */
#define cd_hash_init(hashtable) \
	__cd_hash_init(hashtable, CD_HASH_SIZE(hashtable))

/* @brief	Add an object to a hashtable.
 * @hashtable: hashtable to add to
 * @node: the &struct cd_hlist_node of the object to be added
 * @key: the key of the object to be added */
#define cd_hash_add(hashtable, node, key)	\
	cd_hlist_add_head(node, \
		&hashtable[cd_hash_min(key, CD_HASH_BITS(hashtable))])

#define cd_hash_add_bits(hashtable, node, key, bits)	\
	cd_hlist_add_head(node, \
		&hashtable[cd_hash_min(key, bits)])

/* @brief	Check whether an object is in any hashtable.
 * @node: the &struct cd_hlist_node of the object to be checked */
static int
cd_hash_hashed(struct cd_hlist_node *node) {
	return !cd_hlist_unhashed(node);
}

static int
__cd_hash_empty(struct cd_hlist_head *ht, unsigned int sz) {
	unsigned int i;

	for (i = 0; i < sz; i++)
		if (!cd_hlist_empty(&ht[i]))
			return -1;

	return 0;
}

/* @brief	Check whether a cd_hashtable is empty.
 * @details	This has to be a macro since HASH_BITS() will not work on pointers since
 * it calculates the size during preprocessing.
 * @hashtable: hashtable to check */
#define cd_hash_empty(hashtable) \
	__cd_hash_empty(hashtable, CD_HASH_SIZE(hashtable))

/* @brief	Remove an object from a hashtable.
 * @node: &struct cd_hlist_node of the object to remove */
static void
cd_hash_del(struct cd_hlist_node *node) {
		cd_hlist_del_init(node);
}

/* @brief	Iterate over a hashtable.
 * @name: hashtable to iterate
 * @bkt: integer to use as bucket loop cursor
 * @obj: the type * to use as a loop cursor for each entry
 * @member: the name of the cd_hlist_node within the struct */
#define cd_hash_for_each(name, bkt, obj, member)			\
	for ((bkt) = 0, obj = NULL; \
		obj == NULL && (bkt) < CD_HASH_SIZE(name); (bkt)++) \
			cd_hlist_for_each_entry(obj, &name[bkt], member)

/* @brief	Iterate over a hashtable safe against removal of hash entry.
 * @name: hashtable to iterate
 * @bkt: integer to use as bucket loop cursor
 * @tmp: struct cd_hlist_node* used for temporary storage
 * @obj: the type * to use as a loop cursor for each entry
 * @member: the name of the cd_hlist_node within the struct */
#define cd_hash_for_each_safe(name, bkt, tmp, obj, member)			\
	for ((bkt) = 0, obj = NULL; obj == NULL && (bkt) < CD_HASH_SIZE(name); (bkt)++)\
		cd_hlist_for_each_entry_safe(obj, tmp, &name[bkt], member)

/* @brief	Iterate over all possible objects hashing to the same bucket.
 * @name: hashtable to iterate
 * @obj: the type * to use as a loop cursor for each entry
 * @member: the name of the cd_hlist_node within the struct
 * @key: the key of the objects to iterate over */
#define cd_hash_for_each_possible(name, obj, member, key)			\
	cd_hlist_for_each_entry(obj, \
		&name[cd_hash_min(key, CD_HASH_BITS(name))], member)

/* @brief	Iterate over all possible objects hashing to the same bucket safe against removals.
 * @name: hashtable to iterate
 * @obj: the type * to use as a loop cursor for each entry
 * @tmp: struct cd_hlist_node* used for temporary storage
 * @member: the name of the cd_hlist_node within the struct
 * @key: the key of the objects to iterate over */
#define cd_hash_for_each_possible_safe(name, obj, tmp, member, key)	\
	cd_hlist_for_each_entry_safe(obj, tmp,\
		&name[cd_hash_min(key, CD_HASH_BITS(name))], member)


#endif	/* CD_HASHTBALE_H */
