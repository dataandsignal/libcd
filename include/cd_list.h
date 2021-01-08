/**
 * cd_list.h - Doubly linked list implementation, based on Linux kernel implementation
 *
 * Part of the libcd - bringing you support for C programs with queue processors, from Data And Signal's Piotr Gregor
 * 
 * Data And Signal - IT Solutions
 * http://www.dataandsignal.com
 * 2020
 *
 */


#ifndef CD_LIST_H
#define CD_LIST_H


#include <stdlib.h>
#include <limits.h>
#include <stddef.h>


#define cd_container_of(ptr, type, member) __extension__ ({         \
        const __typeof__( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})


/* move the poison pointer offset into some well-recognized area such as 0xdead000000000000 */
# define POISON_POINTER_DELTA 0

/* These are non-NULL pointers that will result in page faults */
#define CD_LIST_POISON1  (void*)((unsigned char*) 0x00100100 + POISON_POINTER_DELTA)
#define CD_LIST_POISON2  (void*)((unsigned char*) 0x00200200 + POISON_POINTER_DELTA)
 
#ifndef CONFIG_DEBUG_LIST
struct cd_list_head
{
	struct cd_list_head	*next, *prev;
};

struct cd_hlist_head
{
	struct cd_hlist_node	*first;
};

struct cd_hlist_node
{
	struct cd_hlist_node	*next, **pprev;
};
#else
typedef cd_list_head_dbg cd_list_head;
typedef cd_hlist_head_dbg cd_hlist_head;
typedef cd_hlist_node_dbg cd_hlist_node;
#endif

#define CD_LIST_HEAD_INIT(name) { &(name), &(name) }
#define CD_LIST_HEAD(name) struct cd_list_head name = CD_LIST_HEAD_INIT(name)

static void CD_INIT_LIST_HEAD(struct cd_list_head *list)
{
	list->next = list;
	list->prev = list;
}

/* Insert a new entry between two known consecutive entries.
 * This is only for internal list manipulation where we know
 * the prev/next entries already! */
#ifndef CONFIG_DEBUG_LIST
static void __cd_list_add(struct cd_list_head *new,
		struct cd_list_head *prev,
		struct cd_list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}
#else
extern void __cd_list_add(struct cd_list_head *new,
		struct cd_list_head *prev,
		struct cd_list_head *next);
#endif


/* @brief   Add a new entry after the specified head.
 * @param   new:	new entry to be added
 * @param   head:	list head to add it after
 * @details	This is good for implementing stacks. */
static void cd_list_add(struct cd_list_head *new,
				struct cd_list_head *head)
{
		__cd_list_add(new, head, head->next);
}


/* @brief       Add a new entry before the specified head.
 * @param       new:	new entry to be added
 * @param       head:	list head to add it before
 * @details     This is useful for implementing queues. */
static void cd_list_add_tail(struct cd_list_head *new,
				struct cd_list_head *head)
{
		__cd_list_add(new, head->prev, head);
}

/* @brief   Delete a list entry by making the prev/next entries
 *          point to each other.
 * @details This is only for internal list manipulation where we know
 *          the prev/next entries already! */
static void __cd_list_del(struct cd_list_head * prev,
				struct cd_list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

/* @brief   Deletes entry from list.
 * @param   entry: the element to delete from the list.
 * @details list_empty() on entry does not return
 *          true after this, the entry is in an undefined state. */
#ifndef CONFIG_DEBUG_LIST
static void __cd_list_del_entry(struct cd_list_head *entry)
{
	__cd_list_del(entry->prev, entry->next);
}

static void cd_list_del(struct cd_list_head *entry)
{
	__cd_list_del(entry->prev, entry->next);
	entry->next = CD_LIST_POISON1;
	entry->prev = CD_LIST_POISON2;
}
#else
extern void __cd_list_del_entry(struct cd_list_head *entry);
extern void cd_list_del(struct cd_list_head *entry);
#endif

/* @brief   Deletes entry from cd_list and reinitializes it.
 * @param   entry: the element to delete from the cd_list. */
static void cd_list_del_init(struct cd_list_head *entry)
{
	__cd_list_del_entry(entry);
	CD_INIT_LIST_HEAD(entry);
}

/* @brief   Delete from one cd_list and add as another's head
 * @param   cd_list: the entry to move
 * @param   head: the head that will precede our entry */
static void cd_list_move(struct cd_list_head *cd_list, struct cd_list_head *head)
{
	__cd_list_del_entry(cd_list);
	cd_list_add(cd_list, head);
}

/* @brief	Delete from one cd_list and add as another's tail
 * @cd_list: the entry to move
 * @head: the head that will follow our entry */
static void cd_list_move_tail(struct cd_list_head *cd_list,
						  struct cd_list_head *head)
{
	__cd_list_del_entry(cd_list);
	cd_list_add_tail(cd_list, head);
}

/* @brief	Tests whether @list is the last entry in cd_list @head
 * @list: the entry to test
 * @head: the head of the cd_list which @list is member of */
static int cd_list_is_last(const struct cd_list_head *list,
					const struct cd_list_head *head)
{
	return list->next == head;
}

/* @brief	Tests whether a cd_list is empty.
 * @head: the cd_list to test. */
static int cd_list_empty(const struct cd_list_head *head)
{
	return head->next == head;
}

/* @brief	Tests whether a cd_list is empty and not being modified
 * @head: the cd_list to test
 * @details	Tests whether a cd_list is empty _and_ checks that
 * no other CPU might be in the process of modifying either
 * member (next or prev)
 * NOTE: using cd_list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the cd_list entry is cd_list_del_init(). Eg. it
 * cannot be used if another CPU could re-cd_list_add() it. */
static int cd_list_empty_careful(const struct cd_list_head *head)
{
		struct cd_list_head *next = head->next;
			return (next == head) && (next == head->prev);
}

/* @brief	Rotate the cd_list to the left
 * @head: the head of the cd_list */
static void cd_list_rotate_left(struct cd_list_head *head)
{
	struct cd_list_head *first;
	if (!cd_list_empty(head))
	{
		first = head->next;
		cd_list_move_tail(first, head);
	}
}

/* @brief	Tests whether a cd_list has just one entry.
 * @head: the cd_list to test. */
static int cd_list_is_singular(const struct cd_list_head *head)
{
	return !cd_list_empty(head) && (head->next == head->prev);
}

static void __cd_list_cut_position(struct cd_list_head *cd_list,
						struct cd_list_head *head,
						struct cd_list_head *entry)
{
	struct cd_list_head *new_first = entry->next;
	cd_list->next = head->next;
	cd_list->next->prev = cd_list;
	cd_list->prev = entry;
	entry->next = cd_list;
	head->next = new_first;
	new_first->prev = head;
}

/* @brief	Cut a cd_list into two.
 * @cd_list: a new cd_list to add all removed entries
 * @head: a cd_list with entries
 * @entry: an entry within head, could be the head itself
 * and if so we won't cut the cd_list
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @cd_list. You should
 * pass on @entry an element you know is on @head. @cd_list
 * should be an empty cd_list or a cd_list you do not
 * care about losing its data. */
static void cd_list_cut_position(struct cd_list_head *cd_list,
						struct cd_list_head *head,
						struct cd_list_head *entry)
{
	if (cd_list_empty(head))
		return;
	if (cd_list_is_singular(head) &&
		(head->next != entry && head != entry))
	return;
	if (entry == head)
		CD_INIT_LIST_HEAD(cd_list);
	else
		__cd_list_cut_position(cd_list, head, entry);
}

static void __cd_list_splice(const struct cd_list_head *cd_list,
						struct cd_list_head *prev,
						struct cd_list_head *next)
{
	struct cd_list_head *first = cd_list->next;
	struct cd_list_head *last = cd_list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/* @brief	Join two lists, this is designed for stacks.
 * @cd_list: the new list to add.
 * @head: the place to add it in the first list. */
static void cd_list_splice(const struct cd_list_head *cd_list,
						struct cd_list_head *head)
{
	if (!cd_list_empty(cd_list))
		__cd_list_splice(cd_list, head, head->next);
}

/* @brief	Join two lists, each list being a queue.
 * @cd_list: the new list to add.
 * @head: the place to add it in the first list. */
static void cd_list_splice_tail(struct cd_list_head *cd_list,
						struct cd_list_head *head)
{
	if (!cd_list_empty(cd_list))
		__cd_list_splice(cd_list, head->prev, head);
}

/* @brief	Join two cd_lists and reinitialise the emptied list.
 * @cd_list: the new list to add.
 * @head: the place to add it in the first list.
 * @details	The cd_list at @cd_list is reinitialised. */
static void cd_list_splice_init(struct cd_list_head *cd_list,
						struct cd_list_head *head)
{
	if (!cd_list_empty(cd_list))
	{
		__cd_list_splice(cd_list, head, head->next);
		CD_INIT_LIST_HEAD(cd_list);
	}
}

/* @brief	Join two lists and reinitialise the emptied list.
 * @cd_list: the new list to add.
 * @head: the place to add it in the first list.
 * @details	Each of the lists is a queue. The list at @cd_list
 * is reinitialised */
static void cd_list_splice_tail_init(struct cd_list_head *cd_list,
						struct cd_list_head *head)
{
	if (!cd_list_empty(cd_list))
	{
		__cd_list_splice(cd_list, head->prev, head);
		CD_INIT_LIST_HEAD(cd_list);
	}
}

/* @brief	Get the struct for this entry.
 * @ptr:	the &struct cd_list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the cd_list_head within the struct. */
#define cd_list_entry(ptr, type, member) \
		cd_container_of(ptr, type, member)

/* @brief	Get the first element from a cd_list.
 * @ptr:	the cd_list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:the name of the cd_list_head within the struct.
 * @details	Note, that cd_list is expected to be not empty. */
#define cd_list_first_entry(ptr, type, member) \
		cd_list_entry((ptr)->next, type, member)

/* @brief	Get the last element from a cd_list.
 * @ptr:	the cd_list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the cd_list_head within the struct.
 * @details	Note, that cd_list is expected to be not empty. */
#define cd_list_last_entry(ptr, type, member) \
		cd_list_entry((ptr)->prev, type, member)

/* @brief	Get the first element from a cd_list.
 * @ptr:	the cd_list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the cd_list_head within the struct.
 * @details	Note that if the cd_list is empty, it returns NULL. */
#define cd_list_first_entry_or_null(ptr, type, member) \
		(!cd_list_empty(ptr) ? cd_list_first_entry(ptr, type, member) : NULL)

/* @brief	Get the next element in cd_list.
 * @pos:	the type * to cursor
 * @member:	the name of the cd_list_head within the struct. */
#define cd_list_next_entry(pos, member) \
		cd_list_entry((pos)->member.next, __typeof__(*(pos)), member)

/* @brief	Get the prev element in cd_list.
 * @pos:	the type * to cursor
 * @member:	the name of the cd_list_head within the struct. */
#define cd_list_prev_entry(pos, member) \
		cd_list_entry((pos)->member.prev, __typeof__(*(pos)), member)

/* @brief	Iterate over a cd_list.
 * @pos:	the &struct cd_list_head to use as a loop cursor.
 * @head:	the head for your cd_list. */
#define cd_list_for_each(pos, head) \
		for (pos = (head)->next; pos != (head); pos = pos->next)

/* @brief	Iterate over a cd_list backwards.
 * @pos:	the &struct cd_list_head to use as a loop cursor.
 * @head:	the head for your cd_list. */
#define cd_list_for_each_prev(pos, head) \
		for (pos = (head)->prev; pos != (head); pos = pos->prev)

/* @brief	Iterate over a cd_list safe against removal of cd_list entry.
 * @pos:	the &struct cd_list_head to use as a loop cursor.
 * @n:	another &struct cd_list_head to use as
 * temporary storage
 * @head:	the head for your cd_list. */
#define cd_list_for_each_safe(pos, n, head) \
		for (pos = (head)->next, n = pos->next; pos != (head); \
						pos = n, n = pos->next)

/* @brief	Iterate over a cd_list backwards safe against removal
 * of cd_list entry.
 * @pos:    the &struct cd_list_head to use as a loop cursor
 * @n:  another &struct cd_list_head to use as temporary storage
 * @head:   the head for your cd_list */
#define cd_list_for_each_prev_safe(pos, n, head) 			\
		for (pos = (head)->prev, n = pos->prev; 		\
					     pos != (head); 		\
					     pos = n, n = pos->prev)

/* @brief	Iterate over cd_list of given type
 * @pos:	the type * to use as a loop cursor
 * @head:	the head for your cd_list
 * @member:the name of the cd_list_head within the struct */
#define cd_list_for_each_entry(pos, head, member)				\
		for (pos = cd_list_first_entry(head, __typeof__(*pos), member);	\
					     &pos->member != (head);		\
					     pos = cd_list_next_entry(pos, member))

/* @brief	Iterate backwards over cd_list of given type.
 * @pos:	the type * to use as a loop cursor
 * @head:	the head for your cd_list
 * @member:the name of the cd_list_head within the struct */
#define cd_list_for_each_entry_reverse(pos, head, member)			\
		for (pos = cd_list_last_entry(head, __typeof__(*pos), member);	\
					     &pos->member != (head); 		\
					     pos = cd_list_prev_entry(pos, member))

/* @brief	Prepare a pos entry for use in cd_list_for_each_entry_continue().
 * @pos:	the type * to use as a start point
 * @head:	the head of the cd_list
 * @member:the name of the cd_list_head within the struct
 * @details	Prepares a pos entry for use as a start point in
 * cd_list_for_each_entry_continue(). */
#define cd_list_prepare_entry(pos, head, member) \
		((pos) ? : cd_list_entry(head, __typeof__(*pos), member))

/* @brief	Continue iteration over cd_list of given type.
 * @pos:	the type * to use as a loop cursor
 * @head:	the head for your cd_list
 * @member:the name of the cd_list_head within the struct
 * Continue to iterate over cd_list of given type, continuing after
 * the current position. */
#define cd_list_for_each_entry_continue(pos, head, member) 			\
		for (pos = cd_list_next_entry(pos, member);			\
					     &pos->member != (head);		\
					     pos = cd_list_next_entry(pos, member))

/* @brief	Iterate backwards from the given point.
 * @pos:	the type * to use as a loop cursor
 * @head:	the head for your list
 * @member:the name of the cd_list_head within the struct
 * @details	Start to iterate over list of given type backwards, continuing after
 * the current position. */
#define cd_list_for_each_entry_continue_reverse(pos, head, member)		\
		for (pos = cd_list_prev_entry(pos, member);			\
					     &pos->member != (head);		\
					     pos = cd_list_prev_entry(pos, member))

/* @brief	Iterate over cd_list of given type from the current point.
 * @pos:	the type * to use as a loop cursor
 * @head:	the head for your cd_list
 * @member:the name of the cd_list_head within the struct
 * @details	Iterate over list of given type, continuing from current position. */
#define cd_list_for_each_entry_from(pos, head, member) 			\
		for (; &pos->member != (head);					\
					     pos = cd_list_next_entry(pos, member))

/* @brief	Iterate over cd_list of given type safe against removal
 * of cd_list entry.
 * @pos:	the type * to use as a loop cursor
 * @n:	another type * to use as temporary storage
 * @head:	the head for your list
 * @member:the name of the cd_list_head within the struct. */
#define cd_list_for_each_entry_safe(pos, n, head, member)			\
		for (pos = cd_list_first_entry(head, __typeof__(*pos), member),	\
			n = cd_list_next_entry(pos, member);			\
			&pos->member != (head); 				\
			pos = n, n = cd_list_next_entry(n, member))

/* @brief	Continue cd_list iteration safe against removal.
 * @pos:	the type * to use as a loop cursor
 * @n:	another type * to use as temporary storage
 * @head:	the head for your cd_list
 * @member:the name of the list head within the struct
 * @details	Iterate over cd_list of given type, continuing after current point,
 * safe against removal of list entry. */
#define cd_list_for_each_entry_safe_continue(pos, n, head, member) 		\
		for (pos = cd_list_next_entry(pos, member), 			\
			n = cd_list_next_entry(pos, member);			\
			&pos->member != (head);					\
			pos = n, n = cd_list_next_entry(n, member))

/* @brief	Iterate over cd_list from current point safe against removal.
 * @pos:	the type * to use as a loop cursor
 * @n:	another type * to use as temporary storage
 * @head:	the head for your cd_list
 * @member:the name of the cd_list_head within the struct
 * @details	Iterate over cd_list of given type from current point, safe against
 * removal of cd_list entry. */
#define cd_list_for_each_entry_safe_from(pos, n, head, member) 	\
	for (n = cd_list_next_entry(pos, member);			\
		&pos->member != (head);					\
		pos = n, n = cd_list_next_entry(n, member))

/* @brief	Iterate backwards over cd_list safe against removal.
 * @pos:	the type * to use as a loop cursor
 * @n:	another type * to use as temporary storage
 * @head:	the head for your cd_list
 * @member:the name of the cd_list_head within the struct
 * @details	Iterate backwards over cd_list of given type, safe against removal
 * of cd_list entry. */
#define cd_list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = cd_list_last_entry(head, __typeof__(*pos), member),		\
		n = cd_list_prev_entry(pos, member);				\
		&pos->member != (head); 					\
		pos = n, n = cd_list_prev_entry(n, member))

/* @brief	Double linked lists with a single pointer list head.
 * @details	Mostly useful for hash tables where the two pointer list
 * head is too wasteful. You lose the ability to access
 * the tail in O(1). */

#define CD_HLIST_HEAD_INIT { .first = NULL }
#define CD_HLIST_HEAD(name) struct cd_hlist_head name = {  .first = NULL }
#define CD_INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static void CD_INIT_HLIST_NODE(struct cd_hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static int cd_hlist_unhashed(const struct cd_hlist_node *h)
{
	return !h->pprev;
}

static int cd_hlist_empty(const struct cd_hlist_head *h)
{
	return !h->first;
}

static void __cd_hlist_del(struct cd_hlist_node *n)
{
	struct cd_hlist_node *next = n->next;
	struct cd_hlist_node **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static void cd_hlist_del(struct cd_hlist_node *n)
{
	__cd_hlist_del(n);
	n->next = CD_LIST_POISON1;
	n->pprev = CD_LIST_POISON2;
}

static void cd_hlist_del_init(struct cd_hlist_node *n)
{
	if (!cd_hlist_unhashed(n))
	{
		__cd_hlist_del(n);
		CD_INIT_HLIST_NODE(n);
	}
}

static void cd_hlist_add_head(struct cd_hlist_node *n,
			struct cd_hlist_head *h)
{
	struct cd_hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

/* next must be != NULL */
static void cd_hlist_add_before(struct cd_hlist_node *n,
				struct cd_hlist_node *next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}

static void cd_hlist_add_after(struct cd_hlist_node *n,
			struct cd_hlist_node *next)
{
	next->next = n->next;
	n->next = next;
	next->pprev = &n->next;

	if(next->next)
		next->next->pprev  = &next->next;
}

/* after that we'll appear to be on some hlist and hlist_del will work */
static void cd_hlist_add_fake(struct cd_hlist_node *n)
{
	n->pprev = &n->next;
}

/* Move a list from one list head to another. Fixup the pprev
 * reference of the first entry if it exists. */
static void cd_hlist_move_list(struct cd_hlist_head *old,
			struct cd_hlist_head *new)
{
	new->first = old->first;
	if (new->first)
		new->first->pprev = &new->first;
	old->first = NULL;
}

#define cd_hlist_entry(ptr, type, member) \
	cd_container_of(ptr,type,member)

#define cd_hlist_for_each(pos, head) \
	for (pos = (head)->first; pos ; pos = pos->next)

#define cd_hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
		pos = n)

#define cd_hlist_entry_safe(ptr, type, member) __extension__\
	({ __typeof__(ptr) ____ptr = (ptr); \
		____ptr ? cd_hlist_entry(____ptr, type, member) : NULL; \
	})

/* @brief	Iterate over list of given type.
 * @pos:		the type * to use as a loop cursor.
 * @head:		the head for your list.
 * @member:	the name of the hlist_node within the struct. */
#define cd_hlist_for_each_entry(pos, head, member)				\
	for (pos = cd_hlist_entry_safe((head)->first, __typeof__(*(pos)), member);\
		pos;							\
		pos = cd_hlist_entry_safe((pos)->member.next, 	\
		__typeof__(*(pos)), member))

/* @brief	Iterate over a cd_hlist continuing after current point.
 * @pos:		the type * to use as a loop cursor.
 * @member:	the name of the cd_hlist_node within the struct. */
#define cd_hlist_for_each_entry_continue(pos, member)			\
	for (pos = hlist_entry_safe((pos)->member.next, __typeof__(*(pos)), member);\
		pos;								\
		pos = cd_hlist_entry_safe((pos)->member.next, 		\
			__typeof__(*(pos)), member))

/* @brief	Iterate over a cd_hlist continuing from current point.
 * @pos:		the type * to use as a loop cursor.
 * @member:	the name of the hlist_node within the struct. */
#define cd_hlist_for_each_entry_from(pos, member)				\
	for (; pos;								\
	pos = cd_hlist_entry_safe((pos)->member.next, __typeof__(*(pos)), member))

/* @brief	Iterate over list of given type safe against removal of list entry.
 * @pos:		the type * to use as a loop cursor.
 * @n:		another &struct hlist_node to use as temporary storage
 * @head:		the head for your list.
 * @member:	the name of the hlist_node within the struct. */
#define cd_hlist_for_each_entry_safe(pos, n, head, member) 		\
		for (pos = cd_hlist_entry_safe((head)->first, __typeof__(*pos), member);\
			pos && (n = pos->member.next, 1);			\
			pos = cd_hlist_entry_safe(n, __typeof__(*pos), member))

/* @brief	First-in, first-out queue. */
typedef struct cd_list_head cd_fifo_queue;

static void cd_fifo_enqueue(struct cd_list_head *new, cd_fifo_queue *q) {
	cd_list_add_tail(new, (struct cd_list_head *)q);
}

static struct cd_list_head* cd_fifo_dequeue_f(cd_fifo_queue *q)
{
	struct cd_list_head *l; 
	if (cd_list_empty((struct cd_list_head*)q))
		return NULL;
	l = q->next;
	cd_list_del(l);
	return l;
}
#define cd_fifo_dequeue(q,l) __extension__   \
	({ cd_list_empty((struct cd_list_head*)q) ? \
     l = NULL : (l = ((struct cd_list_head*)q)->next, cd_list_del(l), l); \
	})

/* @brief   Get pointer to an entry BUT not dequeue it. */
#define cd_fifo_get_entry(q, type, member) __extension__ {(		\
	({ (cd_list_empty((struct cd_list_head *)q)) ?	\
		NULL;						\
		cd_list_first_entry((struct cd_list_head *)q,\
					 type, member) })	\

#define cd_fifo_empty(q) \
	cd_list_empty((struct cd_list_head *)q)
#endif /* CD_LIST_H */
