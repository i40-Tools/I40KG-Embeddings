/**
 * Copyright (C) 2011-2015 Universidad Simón Bolívar
 *
 * @brief Wrappers for the C libraries allocation functions
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gpalma@ldc.usb.ve>
 */

static inline void *xmalloc(size_t size)
{
     void *ptr = malloc(size);
     if (!ptr) {
	  fprintf(stderr, "Out of memory, malloc failed tried to allocate %u bytes\n",
		  (unsigned int)size);
	  exit(1);
     }
     return ptr;
}

static inline void *xrealloc(void *old_ptr, size_t size)
{
     void *ptr = realloc(old_ptr, size);
     if (!ptr) {
	  fprintf(stderr, "Out of memory, realloc\n");
	  exit(1);
     }
     return ptr;
}

static inline void *xcalloc(size_t nmemb, size_t size)
{
     void *ptr = calloc(nmemb, size);
     if (!ptr) {
	  fprintf(stderr, "Out of memory, calloc failed\n");
	  exit(1);
     }
     return ptr;
}

/*
 * Realloc the buffer pointed at by variable 'x' so that it can hold
 * at least 'nr' entries; the number of entries currently allocated
 * is 'alloc', using a growing factor.
 *
 * DO NOT USE any expression with side-effect for 'x', 'nr', or 'alloc'.
 */
#define ALLOC_GROW(x, nr, alloc)		    \
     do {					    \
	  if ((nr) > alloc) {				 \
	       if (((alloc+16)*3/2) < (nr))		 \
		    alloc = (nr);			 \
	       else						\
		    alloc = ((alloc+16)*3/2);			\
	       (x) = xrealloc((x), (alloc) * sizeof(*(x)));	\
	  }							\
     } while (0)


#define ARRAY_PUSH(v, e)				\
     do {						\
	  ALLOC_GROW((v).data, (v).nr+1, (v).alloc);	\
	  (v).data[(v).nr++] = (e);			\
     } while(0)

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


/*
#define ALLOC_STRUCT(v)					\
     do {						\
	  typeof(*v) my_szero_ ## v = {0, 0, NULL};	\
	       v = xmalloc(sizeof *v);			\
	       *v = my_szero_ ## v;			\
     } while (0)
*/
