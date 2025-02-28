#ifndef TINC_LIST_H
#define TINC_LIST_H

/*
    list.h -- header file for list.c
    Copyright (C) 2000-2005 Ivo Timmermans
                  2000-2012 Guus Sliepen <guus@tinc-vpn.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

typedef struct list_node_t {
	struct list_node_t *prev;
	struct list_node_t *next;

	/* Payload */

	void *data;
} list_node_t;

typedef void (*list_action_t)(const void *data);
typedef void (*list_action_node_t)(const list_node_t *node);

typedef struct list_t {
	list_node_t *head;
	list_node_t *tail;
	int count;

	/* Callbacks */

	list_action_t delete;
} list_t;

/* (De)constructors */

extern list_t *list_alloc(list_action_t delete) ATTR_MALLOC;
extern void list_free(list_t *list);
extern list_node_t *list_alloc_node(void);
extern void list_free_node(list_t *list, list_node_t *node);

/* Insertion and deletion */

extern list_node_t *list_insert_head(list_t *list, void *data);
extern list_node_t *list_insert_tail(list_t *list, void *data);
extern list_node_t *list_insert_after(list_t *list, list_node_t *node, void *data);
extern list_node_t *list_insert_before(list_t *list, list_node_t *node, void *data);

extern void list_empty_list(list_t *list);
extern void list_delete(list_t *list, const void *data);

extern void list_unlink_node(list_t *list, list_node_t *node);
extern void list_delete_node(list_t *list, list_node_t *node);

extern void list_delete_head(list_t *list);
extern void list_delete_tail(list_t *list);

/* Head/tail lookup */

extern void *list_get_head(list_t *list);
extern void *list_get_tail(list_t *list);

/* Fast list deletion */

extern void list_delete_list(list_t *list);

/* Traversing */

extern void list_foreach(list_t *list, list_action_t action);
extern void list_foreach_node(list_t *list, list_action_node_t action);

/*
   Iterates over a list.

   CAUTION: while this construct supports deleting the current item,
   it does *not* support deleting *other* nodes while iterating on the list.
 */
#define list_each(type, item, list) (type *item = (type *)1; item; item = NULL) for(list_node_t *node = (list)->head, *next; item = node ? node->data : NULL, next = node ? node->next : NULL, node; node = next)

#endif
