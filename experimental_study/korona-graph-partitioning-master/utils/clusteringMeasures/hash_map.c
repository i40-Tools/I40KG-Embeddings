/**
 * Copyright 2010-2015 (C), Universidad Simon Bolivar
 *
 * @brief Generic hash table
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gpalma@ldc.usb.ve>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*#include <stdint.h> */
#include <assert.h>

#include "hash_map.h"

#define HASH_PRIMES     31
#define LOAD_FACTOR     0.80

static const uint32_t primes[HASH_PRIMES] =
{
     3ul,          11ul,         23ul,         53ul,         97ul,
     193ul,        389ul,        769ul,        1543ul,       3079ul,
     6151ul,       12289ul,      24593ul,      49157ul,      98317ul,
     196613ul,     393241ul,     786433ul,     1572869ul,    3145739ul,
     6291469ul,    12582917ul,   25165843ul,   50331653ul,   100663319ul,
     201326611ul,  402653189ul,  805306457ul,  1610612741ul, 3221225473ul,
     4294967291ul
};

static void rehash(struct hash_map *h);
static void rehash_int(struct hash_map *h);

static inline int hmap_hash_code(const struct hash_map *ht, const char *key,
				 unsigned int len, uint32_t *h)
{
     *h = __hash_function(key, len);
     return ( *h % ht->buckets);
}

static inline unsigned int hmap_hash_code_int(const struct hash_map *ht,
					      unsigned int key, unsigned int *h)
{
     *h = __hash_function_int(key);
     return (*h % ht->buckets);
}

static inline int hmap_entry_init(struct hash_entry *e, const char *key_str,
				  unsigned int len)
{
     INIT_HLIST_NODE(&(e->head));
     if (key_str){
	  if ((e->key = (char *)malloc(len+1)) == NULL)
	       return -1;
	  strcpy((char *)e->key, (char *)key_str);
	  /*		memcpy(e->key, key_str, len); */
	  e->keylen = len;
     }
     return 0;
}

static inline int hmap_entry_init_int(struct hash_item *e, unsigned int key)
{
     INIT_HLIST_NODE(&(e->head));
     e->key =  key;

     return 0;
}

int hmap_create(struct hash_map *ht, unsigned int size)
{
     unsigned int i;

     if (size == 0) {
	  ht->buckets = primes[0];
	  ht->next_prime = 1;
     } else if (size >= primes[HASH_PRIMES-1]) {
	  ht->buckets = primes[HASH_PRIMES-1];
	  ht->next_prime = HASH_PRIMES-1;
     } else {
	  ht->buckets = size;
	  i = 0;
	  ht->next_prime = i;
	  while(primes[i] < size){
	       i++;
	       ht->next_prime = i;
	  }
     }
     ht->fill = 0;
     if ((ht->table = (struct hlist_head *)malloc(sizeof(struct hlist_head)*ht->buckets)) == NULL)
	  return -1;

     for (i = 0; i < ht->buckets; i++)
	  INIT_HLIST_HEAD(&ht->table[i]);

     return 0;
}

void hmap_destroy(struct hash_map *ht)
{
     if (ht->table)
	  free(ht->table);
     ht->buckets = 0;
     ht->fill = 0;
     ht->next_prime = 0;
}

int hmap_add(struct hash_map *ht, struct hash_entry *e,
	     const char *key_str, unsigned int len)
{
     unsigned int hashed_key;
     struct hlist_head *h_list;
     float alpha;
     int result;

     alpha = (float)ht->fill/ht->buckets;
     if (alpha >= LOAD_FACTOR){
	  rehash(ht);
     }
     if ( (result = hmap_entry_init(e,key_str,len)) != 0 )
	  return result;
     hashed_key = hmap_hash_code(ht, key_str, len, &e->hash);
     h_list = &ht->table[hashed_key];
     hlist_add_head(&e->head, h_list);
     ht->fill++;

     return result;
}

int hmap_add_int(struct hash_map *ht, struct hash_item *e,
		 unsigned int key)
{
     unsigned int hashed_key;
     struct hlist_head *h_list;
     float alpha;

     alpha = (float)ht->fill/ht->buckets;
     if (alpha >= LOAD_FACTOR){
	  rehash_int(ht);
     }

     hmap_entry_init_int(e, key);

     hashed_key = hmap_hash_code_int(ht, key, &e->hash);
     h_list = &ht->table[hashed_key];
     hlist_add_head(&e->head, h_list);
     ht->fill++;

     return 0;
}

struct hash_entry *hmap_find_member(const struct hash_map *ht,
				    const char *key_str, unsigned int len)
{
     struct hash_entry *entry;
     struct hlist_head *h_list;
     struct hlist_node *list;
     unsigned int hashed_key;
     uint32_t hash;

     hashed_key = hmap_hash_code(ht, key_str, len, &hash);
     h_list = &ht->table[hashed_key];
     hlist_for_each(list, h_list) {
	  entry = hlist_entry(list, struct hash_entry, head);
	  //		if ((entry->keylen == len) && !memcmp(key_str, entry->key, len))
	  if ((entry->keylen == len) && !strncmp(key_str, entry->key, len))
	       return entry;
     }
     return NULL;
}

struct hash_item *hmap_find_member_int(const struct hash_map *ht,
				       unsigned int key)
{
     struct hash_item *entry;
     struct hlist_head *h_list;
     struct hlist_node *list;
     unsigned int hashed_key;
     unsigned int hash;

     hashed_key = hmap_hash_code_int(ht, key, &hash);
     h_list = &ht->table[hashed_key];
     hlist_for_each(list, h_list) {
	  entry = hlist_entry(list, struct hash_item, head);
	  if (entry->key == key)
	       return entry;
     }
     return NULL;
}

int hmap_is_member(const struct hash_map *ht, const char *key_str,
		   unsigned int len)
{
     if (hmap_find_member(ht, key_str, len))
	  return 1;

     return 0;
}

int hmap_is_member_int(const struct hash_map *ht, unsigned int key)
{
     if (hmap_find_member_int(ht, key))
	  return 1;

     return 0;
}

struct hash_entry *hmap_add_if_not_member(struct hash_map *ht, struct hash_entry *e,
					  const char *key_str, unsigned int len)
{
     struct hash_entry *entry;
     struct hlist_head *h_list;
     struct hlist_node *list;
     unsigned int hashed_key;
     float alpha;

     hashed_key = hmap_hash_code(ht, key_str, len, &e->hash);
     h_list = &ht->table[hashed_key];
     hlist_for_each(list, h_list) {
	  entry = hlist_entry(list, struct hash_entry, head);
	  if ((entry->keylen == len) && !memcmp(key_str, entry->key, len))
	       return entry;
     }
     alpha = (float)ht->fill/ht->buckets;
     if (alpha >= LOAD_FACTOR){
	  rehash(ht);
	  hashed_key = hmap_hash_code(ht, key_str, len, &e->hash);
	  h_list = &ht->table[hashed_key];
     }
     if (hmap_entry_init(e,key_str,len) != 0){
	  fprintf(stderr, "Error to insert in hash table\n");
	  exit(1);
     }
     hlist_add_head(&e->head, h_list);
     ht->fill++;

     return NULL;
}

struct hash_item *hmap_add_if_not_member_int(struct hash_map *ht,
					     struct hash_item *e, unsigned int key)
{
     struct hash_item *entry;
     struct hlist_head *h_list;
     struct hlist_node *list;
     unsigned int hashed_key;
     float alpha;

     hashed_key = hmap_hash_code_int(ht, key, &e->hash);
     h_list = &ht->table[hashed_key];
     hlist_for_each(list, h_list) {
	  entry = hlist_entry(list, struct hash_item, head);
	  if (entry->key == key)
	       return entry;
     }
     alpha = (float)ht->fill/ht->buckets;
     if (alpha >= LOAD_FACTOR){
	  rehash_int(ht);
	  hashed_key = hmap_hash_code_int(ht, key, &e->hash);
	  h_list = &ht->table[hashed_key];
     }
     hmap_entry_init_int(e, key);
     hlist_add_head(&e->head, h_list);
     ht->fill++;

     return NULL;
}

struct hash_entry *hmap_delete_if_member(struct hash_map *ht,
					 const char *key_str, unsigned int len)
{
     struct hash_entry *e;

     if ((e = hmap_find_member(ht, key_str, len)) == NULL)
	  return NULL;

     hlist_del_init(&e->head);
     ht->fill--;

     return e;
}

struct hash_item *hmap_delete_if_member_int(struct hash_map *ht,
					    unsigned int key)
{
     struct hash_item *e;

     if ((e = hmap_find_member_int(ht, key)) == NULL)
	  return NULL;

     hlist_del_init(&e->head);
     ht->fill--;

     return e;
}

void hmap_delete(struct hash_map *ht, struct hash_entry *e)
{
     hlist_del_init(&e->head);
     free(e->key);
     ht->fill--;
}

static void hmap_free(struct hash_map *ht, struct hash_entry *e)
{
     hlist_del_init(&e->head);
     ht->fill--;
}

void hmap_delete_int(struct hash_map *ht, struct hash_item *e)
{
     hlist_del_init(&e->head);
     ht->fill--;
}

static void rehash(struct hash_map *h)
{
     struct hlist_head  all_entry;
     struct hash_entry *hentry;
     struct hlist_head *h_list;
     struct hlist_node *list, *pos;
     unsigned int i;

     if (h->buckets <= 0) {
	  fprintf(stderr, "Error, hash table empty\n");
	  exit(1);
     }

     if (h->buckets < primes[HASH_PRIMES-1]) {
	  INIT_HLIST_HEAD(&all_entry);
	  hmap_for_each_safe(hentry, pos, h) {
	       hmap_free(h, hentry);
	       hlist_add_head(&hentry->head, &all_entry);
	  }
	  h->buckets = primes[h->next_prime];
	  h->next_prime++;
	  if ((h->table = (struct hlist_head *)realloc(h->table, sizeof(struct hlist_head) * h->buckets)) == NULL){
	       fprintf(stderr,"Error, out of memory\n");
	       exit(1);
	  }
	  for (i = 0; i < h->buckets; i++)
	       INIT_HLIST_HEAD(&h->table[i]);
	  hlist_for_each_safe(list,pos, &all_entry){
	       hentry = hlist_entry(list, struct hash_entry, head);
	       hlist_del_init(&hentry->head);
	       if (h->buckets > 0) {
		    h_list = &h->table[hentry->hash%h->buckets];
	       } else {
		    fprintf(stderr,"Error in the number of buckets\n");
		    exit(1);
	       }
	       hlist_add_head(&hentry->head, h_list);
	       h->fill++;
	  }
     }
}

static void rehash_int(struct hash_map *h)
{
     struct hlist_head  all_entry;
     struct hash_item *hentry;
     struct hlist_head *h_list;
     struct hlist_node *list, *pos;
     unsigned int i;

     if (h->buckets <= 0) {
	  fprintf(stderr, "Error, hash table empty\n");
	  exit(1);
     }

     if (h->buckets < primes[HASH_PRIMES-1])  {
	  INIT_HLIST_HEAD(&all_entry);
	  hmap_for_each_safe_int(hentry, pos, h) {
	       hmap_delete_int(h, hentry);
	       hlist_add_head(&hentry->head, &all_entry);
	  }
	  h->buckets = primes[h->next_prime];
	  h->next_prime++;
	  if ((h->table = (struct hlist_head *)realloc(h->table, sizeof(struct hlist_head) * h->buckets)) == NULL){
	       fprintf(stderr, "Error out of memory\n");
	       exit(1);
	  }
	  for (i = 0; i < h->buckets; i++)
	       INIT_HLIST_HEAD(&h->table[i]);

	  hlist_for_each_safe(list,pos, &all_entry){
	       hentry = hlist_entry(list, struct hash_item, head);
	       hlist_del_init(&hentry->head);
	       if (h->buckets > 0) {
		    h_list = &h->table[hentry->hash%h->buckets];
	       } else {
		    fprintf(stderr,"Error in the number of buckets\n");
		    exit(1);
	       }
	       hlist_add_head(&hentry->head, h_list);
	       h->fill++;
	  }
     }
}
