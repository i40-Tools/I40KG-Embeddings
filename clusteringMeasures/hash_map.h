/**
 * Copyright 2010-2015 (C), Universidad Simon Bolivar
 *
 * @brief Generic hash table
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gpalma@ldc.usb.ve>
 */

#ifndef ___HASH_MAP_H
#define ___HASH_MAP_H


#include "dlist.h"
#include "hash_function.h"

struct hash_entry {
     char                *key;
     unsigned int        keylen;
     unsigned int        hash;
     struct hlist_node   head;
};

struct hash_item {
     unsigned int        key;
     unsigned int        hash;
     struct hlist_node   head;
};

struct hash_map {
     unsigned int       fill;
     unsigned int       buckets;
     int                next_prime;
     struct hlist_head  *table;

     /* private variables */
     unsigned int       i;
     struct hlist_node  *pos;
};

/**
 * hash_entry - get the user data for this entry
 * @ptr:	the &struct hash_entry pointer
 * @type:	the type of the user data (e.g. struct my_data) embedded in this entry
 * @member:	the name of the hash_entry within the struct (e.g. entry)
 */
#define hash_entry(_ptr, _type, _member) container_of(_ptr, _type, _member)
/*#define hash_entry(ptr, type, member)								\
  ((type *)((char *)(ptr)-(unsigned int)(&((type *)0)->member)))*/

/**
 * @hentry: &struct hash_entry
 * @htable: &struct hash_table
 */
#define hmap_for_each(hentry, htable)					\
     for	((htable)->i=0; ((htable)->i < (htable)->buckets); ++((htable)->i)) \
	  for ( (htable)->pos = (&((htable)->table[(htable)->i]))->first; \
		(htable)->pos && ({ ; 1; }) && (hentry = hlist_entry((htable)->pos, struct hash_entry, head)); \
		(htable)->pos = ((htable)->pos)->next )

/**
 * @hitem: &struct hash_item
 * @htable: &struct hash_table
 */
#define hmap_for_each_int(hitem, htable)				\
     for	((htable)->i=0; ((htable)->i < (htable)->buckets); ++((htable)->i)) \
	  for ((htable)->pos = (&((htable)->table[(htable)->i]))->first; \
	       (htable)->pos && ({ ; 1; }) && (hitem = hlist_entry((htable)->pos, struct hash_item, head)); \
	       (htable)->pos = ((htable)->pos)->next)

/**
 * @hitem: &struct hash_item
 * @htable: &struct hash_table
 * @n       &struct hlist_node
 */
#define hmap_for_each_safe(hentry, n, htable)				\
     for	((htable)->i=0; ((htable)->i < (htable)->buckets); ++((htable)->i)) \
	  for ((htable)->pos = (&((htable)->table[(htable)->i]))->first; \
	       (htable)->pos && ({ n = ((htable)->pos)->next; 1; }) &&	\
		    (hentry = hlist_entry((htable)->pos, struct hash_entry, head)); \
	       (htable)->pos = n)

/**
 * @hitem: &struct hash_item
 * @htable: &struct hash_table
 * @n       &struct hlist_node
 */
#define hmap_for_each_safe_int(hitem, n, htable)			\
     for	((htable)->i=0; ((htable)->i < (htable)->buckets); ++((htable)->i)) \
	  for ((htable)->pos = (&((htable)->table[(htable)->i]))->first; \
	       (htable)->pos && ({ n = ((htable)->pos)->next; 1; }) &&	\
		    (hitem = hlist_entry((htable)->pos, struct hash_item, head)); \
	       (htable)->pos = n)


#endif /* ___HASH_MAP_H */

int hmap_create(struct hash_map *ht, unsigned int size);

void hmap_destroy(struct hash_map *ht);

int hmap_add(struct hash_map *ht, struct hash_entry *e,
	     const char *key_str, unsigned int len);

int hmap_add_int(struct hash_map *ht, struct hash_item *e,
		 unsigned int key);

struct hash_entry *hmap_find_member(const struct hash_map *ht,
				    const char *key_str, unsigned int len);

struct hash_item *hmap_find_member_int(const struct hash_map *ht,
				       unsigned int key);

int hmap_is_member(const struct hash_map *ht, const char *key_str,
		   unsigned int len);

int hmap_is_member_int(const struct hash_map *ht, unsigned int key);

struct hash_entry *hmap_add_if_not_member(struct hash_map *ht, struct hash_entry *e,
					  const char *key_str, unsigned int len);

struct hash_item *hmap_add_if_not_member_int(struct hash_map *ht,
					     struct hash_item *e, unsigned int key);

struct hash_entry *hmap_delete_if_member(struct hash_map *ht,
					 const char *key_str, unsigned int len);

struct hash_item *hmap_delete_if_member_int(struct hash_map *ht,
					    unsigned int key);

void hmap_delete(struct hash_map *ht, struct hash_entry *e);

void hmap_delete_int(struct hash_map *ht, struct hash_item *e);
