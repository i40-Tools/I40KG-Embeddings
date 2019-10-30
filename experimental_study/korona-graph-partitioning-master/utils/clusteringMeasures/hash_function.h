#ifndef __HASH_FUNCTION_H
#define __HASH_FUNCTION_H

#include <stdint.h>

/**
 * Murmur
 */
static inline unsigned int __hash_function(const char *key, unsigned int len)
{
  /* 'm' and 'r' are mixing constants generated offline.
   * They're not really 'magic', they just happen to work well.
   */
  const unsigned int m = 0x5bd1e995;
  const int r = 24;

  /* Initialize the hash to a 'random' value */
  unsigned int seed = 0x3FB0BB5F;
  unsigned int h = seed ^ len;

  /* Mix 4 bytes at a time into the hash */

  const unsigned char * data = (const unsigned char *)key;

  while(len >= 4)
  {
    unsigned int k = *(unsigned int *)data;

    k *= m;
    k ^= k >> r;
    k *= m;

    h *= m;
    h ^= k;

    data += 4;
    len -= 4;
  }

  /* Handle the last few bytes of the input array */

  switch(len)
  {
              /*  fall through */
    case 3: h ^= data[2] << 16;
	        /*  fall through */
    case 2: h ^= data[1] << 8;
	        /*  fall through */
    case 1: h ^= data[0];
      h *= m;
  };

  /* Do a few final mixes of the hash to ensure the last few
   * bytes are well-incorporated.
   */
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;

  return h;
}


/**
 * FNV Hash Function
 */
/*
  static inline uint32_t __hash_function(const char *data, unsigned int len)
  {
  unsigned int i;
  uint32_t hash;

  hash = UINT32_C(2166136261);
  for (i = 0; i < len; i++) {
  hash = (UINT32_C(16777619) * hash) ^ data[i];
  }
  return hash;
  }
*/

/**
 * X31 Hash Function
 */
/*
  static inline uint32_t __hash_function(const char *s, unsigned int len)
  {
  uint32_t h = *s;
  len = 0;
  if (h)
  for (++s ; *s; ++s)
  h = (h << 5) - h + *s;
  return h;
  }
*/

/**
 * SuperFastHash  by Paul Hsieh (C) 2004, 2005.
 * See http://www.azillionmonkeys.com/qed/hash.html
 */
/*#include "pstdint.h" */
/*
  #undef get16bits
  #if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
  #define get16bits(d) (*((const uint16_t *) (d)))
  #endif

  #if !defined (get16bits)
  #define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
  +(uint32_t)(((const uint8_t *)(d))[0]) )
  #endif

  uint32_t  __hash_function(const char * data, int len) {
  uint32_t hash = len, tmp;
  int rem;

  if (len <= 0 || data == NULL) return 0;

  rem = len & 3;
  len >>= 2;

  for (;len > 0; len--) {
  hash  += get16bits (data);
  tmp    = (get16bits (data+2) << 11) ^ hash;
  hash   = (hash << 16) ^ tmp;
  data  += 2*sizeof (uint16_t);
  hash  += hash >> 11;
  }

  switch (rem) {
  case 3: hash += get16bits (data);
  hash ^= hash << 16;
  hash ^= data[sizeof (uint16_t)] << 18;
  hash += hash >> 11;
  break;
  case 2: hash += get16bits (data);
  hash ^= hash << 11;
  hash += hash >> 17;
  break;
  case 1: hash += *data;
  hash ^= hash << 10;
  hash += hash >> 1;
  }

  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;

  return hash;
  }
*/

/*****************************************************************
 ****************************************************************
 **  Integer Hash function
 **
 ******************************************************************/

/**
 * Robert Jenkins' 32 bit
 * Integer Hash function
 */
/*
static inline uint32_t __hash_function_int(uint32_t a) {
     a = (a+0x7ed55d16) + (a<<12);
     a = (a^0xc761c23c) ^ (a>>19);
     a = (a+0x165667b1) + (a<<5);
     a = (a+0xd3a2646c) ^ (a<<9);
     a = (a+0xfd7046c5) + (a<<3);
     a = (a^0xb55a4f09) ^ (a>>16);
     return a;
}
*/

/**
 * Knuth multiplicative method
 */
/*
static inline unsigned int __hash_function_int(uint32_t key)
{
     return (key >> 3) * 2654435761ul;
}
*/

/**
 * Hashing function for an integer
 * Robert Jenkins' 32 bit Mix Function
 */
/*
static inline unsigned int __hash_function_int(unsigned int key){

     key += (key << 12);
     key ^= (key >> 22);
     key += (key << 4);
     key ^= (key >> 9);
     key += (key << 10);
     key ^= (key >> 2);
     key += (key << 7);
     key ^= (key >> 12);

     // Knuth's Multiplicative Method
     key = (key >> 3) * 2654435761;

     return key;
}
*/

/**
 * Hash fuction by Thomas Wang
 */
/*
static inline uint32_t __hash_function_int(uint32_t key)
{
     key = ~key + (key << 15); // key = (key << 15) - key - 1;
     key = key ^ (key >> 12);
     key = key + (key << 2);
     key = key ^ (key >> 4);
     key = key * 2057; // key = (key + (key << 3)) + (key << 11);
     key = key ^ (key >> 16);
     return key;
}
*/

/**
 * Hash fuction same key
 */

static inline int __hash_function_int(int key)
{
     return key;
}

/**
 * Multiplicative hash function by Thomas Wang
 */
/*
static inline uint32_t __hash_function_int(uint32_t key)
{
*/
// uint32_t c2=0x27d4eb2d; /* a prime or an odd constant */
/*
  key = (key ^ 61) ^ (key >> 16);
  key = key + (key << 3);
  key = key ^ (key >> 4);
  key = key * c2;
  key = key ^ (key >> 15);

  return key;
}
*/


#endif /* __HASH_FUNCTION_H */
