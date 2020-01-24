/**
 * Copyright (C) 2016 Universidad Simón Bolívar
 * 
 * Description: A simple implementation of a set of integers
 *
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gpalma@ldc.usb.ve>
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "hash_iset.h"
#include "memory.h"

#define ALLOW_NR(x) (((x)+16)*3/2)

static int *lookup_hash_iset_entry(const struct hash_iset *s, int hash)
{
     unsigned int n, pos;
     int *array;
     
     n = s->alloc;
     array = s->htable;
     pos = hash % n;

     while (array[pos] != NOELEM) {
	  if (array[pos] == hash)
	       break;
	  pos++;
	  if (pos >= n)
	       pos = 0;
     } 
     return array + pos;
}

void init_hash_iset(struct hash_iset *s)
{
     s->nr = 0;
     s->alloc = 0;
     s->htable = NULL;
     
}

bool lookup_hash_iset(const struct hash_iset *s, int hash)
{
     bool result;

     if (!s->htable) {
	  result = false;
     } else {
	  result = ( *(lookup_hash_iset_entry(s, hash)) == NOELEM ) ? false : true;
     }
     return result;
}

static void grow_set_table(struct hash_iset *s)
{
     unsigned int i, old_size, new_size;
     int *old_array, *new_array, *entry;
     
     old_size = s->alloc;
     old_array = s->htable;
     new_size = ALLOW_NR(old_size);
     new_array = (int *)xmalloc(sizeof(int)*new_size);
     for (i = 0; i < new_size; i++)
	  new_array[i] = NOELEM;
     s->alloc = new_size;
     s->htable = new_array;
     s->nr = 0;
     for (i = 0; i < old_size; i++)
	  if (old_array[i] != NOELEM) {
	       entry = lookup_hash_iset_entry(s, old_array[i]);
	       *entry =  old_array[i];
	       s->nr++;
	  }
     free(old_array);
}

bool insert_hash_iset(struct hash_iset *s, int hash)
{
     unsigned int nr = s->nr;
     int *entry;
     
     if (nr >= s->alloc/2) 
	  grow_set_table(s);
     entry = lookup_hash_iset_entry(s, hash); 
     if (*entry == NOELEM) {
	  *entry = hash;
	   s->nr++;
	  return true;
     }
     return false;
}

void free_hash_iset(struct hash_iset *s)
{
     free(s->htable);
     s->htable = NULL;
     s->alloc = 0;
     s->nr = 0;
}

void print_hash_iset(struct hash_iset *s)
{
     unsigned int i, n, k, alloc;
     int v;

     n = s->nr;
     alloc = s->alloc; 
     i = 0;
     k = 0;
     printf("Number of elements: %u\n", n);
     while ((i < n) && (k < alloc)) {
	  v = s->htable[k++];
	  if (v != NOELEM) {
	       printf("(%u %d) ", i, v);
	       i++;
	  } 
     }
     printf("\n");
}
