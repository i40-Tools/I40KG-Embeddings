/**
 * Copyright (C) 2016 Universidad Simón Bolívar
 * 
 * Description: A simple implementation of a set of integers
 *
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gpalma@ldc.usb.ve>
 */

#ifndef ___HASH_SET_H
#define ___HASH_SET_H

#include <stdio.h>
#include <stdbool.h>

#define NOELEM -1

struct hash_iset {
     unsigned int nr;
     unsigned int alloc;
     int *htable;
};

void init_hash_iset(struct hash_iset *set);

bool lookup_hash_iset(const struct hash_iset *set, int hash);

bool insert_hash_iset(struct hash_iset *set, int hash);

void free_hash_iset(struct hash_iset *s);

void print_hash_iset(struct hash_iset *s);

#endif
