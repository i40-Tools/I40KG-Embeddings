/**
 * Copyright (C) 2012-2015 Universidad Simón Bolívar
 *
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gvpalma@usb.ve>
 */

#ifndef ___UTIL_H
#define ___UTIL_H

#ifdef PRGDEBUG
#define DEBUG(...) printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

#define MIN(x, y) ({                            \
      typeof(x) _min1 = (x);			\
      typeof(y) _min2 = (y);			\
      (void) (&_min1 == &_min2);		\
      _min1 < _min2 ? _min1 : _min2; })

#define MAX(x, y) ({                            \
      typeof(x) _max1 = (x);			\
      typeof(y) _max2 = (y);			\
      (void) (&_max1 == &_max2);		\
      _max1 > _max2 ? _max1 : _max2; })

#define SWAP(a, b)				\
  do { typeof(a) __tmp = (a);                   \
    (a) = (b);                                  \
    (b) = __tmp;				\
  } while (0)

#define init_struct_array(a)			\
     do { (a).nr = 0;				\
	  (a).alloc = 0;			\
	  (a).data = NULL;			\
     } while (0)

#define free_array(a)				\
     do { (a).nr = 0;				\
	  (a).alloc = 0;			\
	  if ((a).data)				\
	       free((a).data);			\
     } while (0)

void fatal(const char *msg, ...);

int error(const char *msg, ...);

double **double_matrix(int nrl, int nrh, int ncl, int nch);

void free_double_matrix(double **m, int nrl, int ncl);

#endif /* ___UTIL_H */
