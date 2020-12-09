/**
 * cd_list_test.c - Unit tests for cd_list
 *
 * Part of the libcd - bringing you support for C programs with queue processors, from Data And Signal's Piotr Gregor
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */

#include "../include/cd_list.h"
#include <assert.h>
#include <stdint.h>


struct test {
	uint32_t	key;
	int			val;
	struct cd_list_head		link;
	struct cd_hlist_head	hlink;
};

static void test_list_head_init(void)
{
	struct cd_list_head	h;

	CD_INIT_LIST_HEAD(&h);
	assert(h.next == h.prev);
	assert(h.prev == &h);
}

static void test_list_add(void)
{
	struct cd_list_head	h, i;
	struct test		g;
	struct test		g1, g2, g3;

	CD_INIT_LIST_HEAD(&h);
	cd_list_add(&g.link, &h);
	assert(h.next == &g.link);
	assert(h.prev == &g.link);
	assert(g.link.next == &h);
	assert(g.link.prev == &h);
	CD_INIT_LIST_HEAD(&i);
	g1.key = 1;
	cd_list_add(&g1.link, &i);
	assert(i.next == &g1.link);
	assert(i.prev == &g1.link);
	assert(g1.link.next == &i);
	assert(g1.link.prev == &i);
	g2.key = 2;
	cd_list_add(&g2.link, &i);
	assert(i.next == &g2.link);
	assert(i.prev == &g1.link);
	assert(g2.link.next == &g1.link);
	assert(g2.link.prev == &i);
	assert(g1.link.next == &i);
	assert(g1.link.prev == &g2.link);
	g3.key = 3;
	cd_list_add(&g3.link, &i);
	assert(i.next == &g3.link);
	assert(i.prev == &g1.link);
	assert(g3.link.next == &g2.link);
	assert(g3.link.prev == &i);
	assert(g2.link.next == &g1.link);
	assert(g2.link.prev == &g3.link);
	assert(g1.link.next == &i);
	assert(g1.link.prev == &g2.link);
}

static void test_list_add_tail(void)
{
	struct cd_list_head	h, i;
	struct test		g;
	struct test		g1, g2, g3;

	CD_INIT_LIST_HEAD(&h);
	cd_list_add_tail(&g.link, &h);
	assert(h.next == &g.link);
	assert(h.prev == &g.link);
	assert(g.link.next == &h);
	assert(g.link.prev == &h);
	CD_INIT_LIST_HEAD(&i);
	g1.key = 1;
	cd_list_add_tail(&g1.link, &i);
	assert(i.next == &g1.link);
	assert(i.prev == &g1.link);
	assert(g1.link.next == &i);
	assert(g1.link.prev == &i);
	g2.key = 2;
	cd_list_add_tail(&g2.link, &i);
	assert(i.next == &g1.link);
	assert(i.prev == &g2.link);
	assert(g2.link.next == &i);
	assert(g2.link.prev == &g1.link);
	assert(g1.link.next == &g2.link);
	assert(g1.link.prev == &i);
	g3.key = 3;
	cd_list_add_tail(&g3.link, &i);
	assert(i.next == &g1.link);
	assert(i.prev == &g3.link);
	assert(g3.link.next == &i);
	assert(g3.link.prev == &g2.link);
	assert(g2.link.next == &g3.link);
	assert(g2.link.prev == &g1.link);
	assert(g1.link.next == &g2.link);
	assert(g1.link.prev == &i);
}


static void test_list_del(void)
{
	struct cd_list_head	h, i, j;
	struct test		g;
	struct test		g1, g2, g3;

	CD_INIT_LIST_HEAD(&h);
	cd_list_del(&h);
	assert(h.next == CD_LIST_POISON1);
	assert(h.prev == CD_LIST_POISON2);
	CD_INIT_LIST_HEAD(&i);
	cd_list_add_tail(&g.link, &i);
	assert(i.next == &g.link);
	assert(i.prev == &g.link);
	assert(g.link.next == &i);
	assert(g.link.prev == &i);
	cd_list_del(&g.link);
	assert(i.next == i.prev);
	assert(i.prev == &i);
	assert(g.link.next == CD_LIST_POISON1);
	assert(g.link.prev == CD_LIST_POISON2);
	CD_INIT_LIST_HEAD(&j);
	g1.key = 1;
	cd_list_add_tail(&g1.link, &j);
	g2.key = 2;
	cd_list_add_tail(&g2.link, &j);
	g3.key = 3;
	cd_list_add_tail(&g3.link, &j);
	assert(g1.link.prev == &j);
	assert(j.next == &g1.link);
	assert(g1.link.next == &g2.link);
	assert(g2.link.prev == &g1.link);
	assert(g2.link.next == &g3.link);
	assert(g3.link.prev == &g2.link);
	assert(g3.link.next == &j);
	cd_list_del(&g2.link);
	assert(g1.link.prev == &j);
	assert(j.next == &g1.link);
	assert(g1.link.next == &g3.link);
	assert(g2.link.prev == CD_LIST_POISON2);
	assert(g2.link.next == CD_LIST_POISON1);
	assert(g3.link.prev == &g1.link);
	assert(g3.link.next == &j);
	cd_list_del(&g1.link);
	assert(g3.link.prev == &j);
	assert(j.next == &g3.link);
	assert(g1.link.prev == CD_LIST_POISON2);
	assert(g1.link.next == CD_LIST_POISON1);
	assert(g3.link.next == &j);
	cd_list_del(&g3.link);
	assert(j.prev == &j);
	assert(j.next == &j);
	assert(g3.link.prev == CD_LIST_POISON2);
	assert(g3.link.next == CD_LIST_POISON1);
}

static void test_list_del_init(void)
{
	struct cd_list_head	h;
	struct test		g;

	CD_INIT_LIST_HEAD(&h);
	cd_list_add_tail(&g.link, &h);
	cd_list_del_init(&g.link);
	assert(h.next == h.prev);
	assert(h.prev == &h);
	assert(g.link.next == g.link.prev);
	assert(g.link.prev == &g.link);
}

static void test_list(void)
{
	test_list_head_init();
	test_list_add();
	test_list_add_tail();
	test_list_del();
	test_list_del_init();
}

int
main(void)
{
	test_list();
	return 0;
}
