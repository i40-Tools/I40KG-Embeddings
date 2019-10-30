/**
 * Copyright (C) 2012-2015 Universidad Simon Bolivar
 *
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gvpalma@usb.ve>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "memory.h"
#include "util.h"

#define BUF         1024
#define END         1
#define BUFSIZE1    256

static void write_out(const char *prefix, const char *msg, va_list params)
{
     char buf[BUF];
     vsnprintf(buf, sizeof(buf), msg, params);
     fprintf(stderr,"%s%s\n", prefix, buf);
}

void fatal(const char *msg, ...)
{
     va_list params;
     
     va_start(params, msg);
     write_out("Fatal: ", msg, params);
     va_end(params);
     
     exit(1);
}

int error(const char *msg, ...)
{
     va_list params;

     va_start(params, msg);
     write_out("Error: ", msg, params);
     va_end(params);

     return -1;
}

/*
  Allocate a double matrix with
  subscript range m[nrl..nrh][ncl..nch]
*/
double **double_matrix(int nrl, int nrh, int ncl, int nch)

{
     int i, nrow, ncol;
     double **m;

     nrow = nrh-nrl+1;
     ncol = nch-ncl+1;

     /* allocate pointers to rows */
     m = (double **) xmalloc((size_t)((nrow+END)*sizeof(double*)));

     m += END;
     m -= nrl;

     /* allocate rows and set pointers to them */
     m[nrl] = (double *) xmalloc((size_t)((nrow*ncol+END)*sizeof(double)));
     m[nrl] += END;
     m[nrl] -= ncl;

     for( i = nrl+1; i <= nrh; i++)
	  m[i] = m[i-1] + ncol;

     /* return pointer to array of pointers to rows */
     return m;
}


void free_double_matrix(double **m, int nrl, int ncl)
{
     free((m[nrl]+ncl-END));
     free((m+nrl-END));
}
