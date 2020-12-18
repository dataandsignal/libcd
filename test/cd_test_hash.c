/**
 * cd_test_hash.c - Unit tests for cd_hash
 *
 * Part of the libcd - bringing you support for C programs with queue processors, from Data And Signal's Piotr Gregor
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */

#include "../include/cd_hash.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>


struct test
{
	uint32_t	key;
	int			val;
};

static void test_hash_init(void)
{
	CD_DEFINE_HASHTABLE(h, 8);
	cd_hash_init(h);

	assert(h[0].first == NULL);
}

static void cd_test_hash(void)
{
	test_hash_init();
}

int main(void)
{
	cd_test_hash();
	printf("COOL!\n");
	return 0;
}
