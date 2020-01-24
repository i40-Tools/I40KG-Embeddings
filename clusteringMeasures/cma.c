/**
 * Copyright (C) 2017 Universidad Simón Bolívar
 *
 * @brief Computing the clustering metrics Conductance,
 *        Cover and Modularity.
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gvpalma@usb.ve>
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dirent.h> 
#include <sys/types.h> 
#include <sys/param.h> 
#include <sys/stat.h> 
#include <unistd.h> 
#include <errno.h>
#include <time.h>
#include <assert.h>
#include <float.h>

#include "memory.h"
#include "hash_map.h"
#include "hash_iset.h"

/************************
* Constants
*************************/

#define BUFSZ           1024
#define END             1
#define NOCLUSTER       -1
#define NOCONDUCTANCE   -1
#define EPSILON         0.000000001
#define ONE             1.000000001
#define LOW_VALUE      -1.0
#define THETA           0.9

const char sep[3] = "--";

/************************
* Macros
*************************/

#define log_err(M, ...) fprintf(stderr, "[ERROR] (file: %s, in line: %d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check(A, M, ...) if(!(A)) { log_err(M, ##__VA_ARGS__); exit(1); }

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

#if !defined MIN
#define MIN(x, y) ({                            \
      typeof(x) _min1 = (x);			\
      typeof(y) _min2 = (y);			\
      (void) (&_min1 == &_min2);		\
      _min1 < _min2 ? _min1 : _min2; })
#endif

/************************
* Structures
*************************/

struct double_array {
     unsigned nr;
     unsigned alloc;
     double *data;
};

struct int_array {
     unsigned nr;
     unsigned alloc;
     int *data;
};

struct square_matrix {
     unsigned int n;
     double **data;
};

struct item_entry {
     unsigned int id;
     struct hash_entry entry;
};

struct cluster {
     char *name;
     char *fname;
     double conductance;
     double degree;
     struct hash_iset vertices;
};

struct cluster_array {
     unsigned nr;
     unsigned alloc;
     struct cluster *data;
};

struct char_array {
     unsigned nr;
     unsigned alloc;
     char *data;
};

/************************
* Utilities
*************************/

static struct int_array iset_to_array(const struct hash_iset *s)
{
     unsigned int i, n, k, alloc;
     int v;
     struct int_array arr;

     init_struct_array(arr);
     n = s->nr;
     alloc = s->alloc; 
     i = 0;
     k = 0;
     while ((i < n) && (k < alloc)) {
	  v = s->htable[k++];
	  if (v != NOELEM) {
	       ARRAY_PUSH(arr, v);
	       i++;
	  }
     }
     return arr;
}

#ifdef PRGDEBUG

static void print_sim_matrix(const struct square_matrix *M)
{
     unsigned int i, j;
     
     printf("***********************\n");
     for (i = 0; i < M->n; i++) {
	  for (j = 0; j < M->n; j++) {
	       printf("%.3f ", M->data[i][j]);
	  }
	  printf("\n");
     }
     printf("***********************\n");
}

static void print_int_array(struct int_array *a)
{
     printf("***********************\n");
     for (unsigned int i = 0; i < a->nr; i++)
	  printf("%d ", a->data[i]);
     printf("\n");
     printf("***********************\n");
}

static void print_double_array(struct double_array *a)
{
     printf("***********************\n");
     for (unsigned int i = 0; i < a->nr; i++)
	  printf("%.3f ", a->data[i]);
     printf("\n");
     printf("***********************\n");
}

static void print_cluter_array(const struct cluster_array *ca)
{
     for (unsigned int i = 0; i < ca->nr; i++) {
	  printf("%d -- %s\n", ca->data[i].id, ca->data[i].fname);
	  struct int_array arr = iset_to_array(&ca->data[i].vertices);
	  print_int_array(&arr);
     }
}

#endif

static inline void init_cluster(struct cluster *c)
{
     c->name = NULL;
     c->fname = NULL;
     c->conductance = NOCONDUCTANCE;
     c->degree = 0.0;
     init_hash_iset(&c->vertices);
}

static inline void init_char_array(struct char_array *buf)
{
     buf->data = xcalloc(BUFSZ, 1);
     buf->alloc = BUFSZ;
     buf->nr = 0;
}

static inline void add_char(struct char_array *buf, char ch)
{
     unsigned alloc, nr;

     alloc = buf->alloc;
     nr = buf->nr;
     if (nr == alloc) {
	  alloc = BUFSZ + alloc;
	  buf->data = xrealloc(buf->data, alloc);
	  buf->alloc = alloc;
     }
     buf->data[nr] = ch;
     buf->nr++;
}

static inline void string_clean(struct char_array *buf)
{
     buf->nr = 0;
}

static void free_cluster_array(struct cluster_array *c)
{
     for (unsigned int i = 0; i < c->nr; i++) {
	  free(c->data[i].name);
	  free(c->data[i].fname);
	  free_hash_iset(&c->data[i].vertices);
     }
     free_array(*c);
}

static inline int cluster_cmp(const void *v1, const void *v2)
{
     const struct cluster *c1 = v1;
     const struct cluster *c2 = v2;

     return  strcmp(c1->name, c2->name);
}

static void free_hash_map_item(struct hash_map *hmap)
{
     struct item_entry *item;
     struct hash_entry *hentry;
     struct hlist_node *n;

     hmap_for_each_safe(hentry, n, hmap) {
	  item = hash_entry(hentry, struct item_entry, entry);
	  hmap_delete(hmap, hentry);
	  free(item);
     }
     hmap_destroy(hmap);
}

/*
 *  Allocate a double matrix with
 *  subscript range m[nrl..nrh][ncl..nch]
 */
static double **double_matrix(int nrl, int nrh, int ncl, int nch)

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

static void free_double_matrix(double **m, int nrl, int ncl)
{
     free((m[nrl]+ncl-END));
     free((m+nrl-END));
}

static double average_conductance(struct cluster_array *ca)
{
     unsigned int i, n;
     double sum;

     sum = 0.0;
     n = ca->nr;
     for (i = 0; i < n; i++)
	  sum += ca->data[i].conductance;
     return sum/n;
}

static double max_conductance(struct cluster_array *ca)
{
     unsigned int i, n;
     double max;

     max = LOW_VALUE;
     n = ca->nr;
     for (i = 0; i < n; i++) {
	  if (max < ca->data[i].conductance)
	       max = ca->data[i].conductance;
     }
     return max;
}

static double min_conductance(struct cluster_array *ca)
{
     unsigned int i, n;
     double minc;

     minc = FLT_MAX;
     n = ca->nr;
     for (i = 0; i < n; i++) {
	  if (minc > ca->data[i].conductance)
	       minc = ca->data[i].conductance;
     }
     return minc;
}

/************************
* Load data from files
*************************/

static void get_name(const char *filename, int n, char *instance)
{
     char buf[n];
     char *aux = NULL;

     strcpy(buf,filename);
     aux = strtok(buf,"/");
     do {
	  strcpy(instance, aux);
     } while((aux = strtok(NULL,"/")) != NULL);

     strcpy(buf,instance);
     aux = strtok(buf,".");
     strcpy(instance, aux);
}

static struct cluster_array get_all_files(const char* dname)
{
     char result[MAXPATHLEN] = {0};
     DIR *d;
     struct dirent *dir;
     int l;
     struct cluster caux;
     struct cluster_array cinput;
	  
     init_struct_array(cinput);
     d = opendir(dname);
     check(d, "Error opening the cluster directory");
     while ((dir = readdir(d)) != NULL){
	  if( strcmp( dir->d_name, "." ) == 0 || 
	      strcmp( dir->d_name, ".." ) == 0 ) {
	       continue;
	  }
	  check((dir->d_type != DT_DIR), "Error, in the directory content");
	  check((getcwd(result, MAXPATHLEN) != NULL), "Error getting cluster path");
	  init_cluster(&caux);
	  l = asprintf(&caux.fname, "%s/%s/%s",result, dname, dir->d_name);
	  check((l != -1), "Error in the file full path");
	  caux.name = xcalloc(l+1, 1);
	  get_name(dir->d_name, l, caux.name);
	  ARRAY_PUSH(cinput, caux);
     }
     closedir(d);
     qsort(cinput.data, cinput.nr, sizeof(struct cluster), cluster_cmp);
     return cinput;
}

static void load_interactions(struct hash_map *v,  const char *fname)
{
     FILE *f;
     char buf[BUFSZ];
     unsigned i, n;
     char *d, *t, *key;
     int l;
     struct item_entry *aux;
          
     f = fopen(fname, "r");
     check(f, "Error reading the interactions file");
     check((fgets(buf, sizeof(buf), f)), "Error reading the number of interactions");
     errno = 0;
     n = strtol(buf, NULL, 10);
     check((errno == 0), "Error in the conversion of string to integer");
     hmap_create(v, n*2);
     
     for (i = 0; i < n; i++) {
	  check((fgets(buf, sizeof(buf), f)), "Error reading the bipartite graph");
	  d = strtok(buf, "\t");
	  t = strtok(NULL, "\t");
	  l = asprintf(&key,"%s%s%s", d, sep, t);
	  check((l != -1), "Error making a key");
	  aux = (struct item_entry *)xmalloc(sizeof(struct item_entry));
	  aux->id = i;
	  check( (hmap_add_if_not_member(v, &aux->entry, key, l) == NULL),
		 "Error, repeated edge in bipartite graph");
	  free(key);
     }
     assert(v->fill == n);
     fclose(f);     
}

static double load_matrix(const char *filename, struct square_matrix *m)
{
     FILE *f;
     struct char_array buf;
     int n, i, j, k;
     int ch;
     double m_avg, v;

     m_avg = 0.0;
     k = 0;
     f = fopen(filename, "r");
     check(f, "No instance file specified, abort");
     n = m->n;
     init_char_array(&buf);
     i = 0;
     j = 0;
     ch = getc(f);
     check((ch != EOF), "Error reading the matrix data file");
     errno = 0;
     while (ch != EOF) {
	  if ((ch != ',') && (ch != '\n')) {
	       add_char(&buf, ch);
	  } else {
	       add_char(&buf, '\0');
	       v = strtod(buf.data, NULL);
	       m->data[i][j] = v;
	       if ( v > EPSILON ) {
		    m_avg += v;
		    k++;
	       }
	       check((errno == 0), "Error in the conversion of string to double");
	       if (ch == ',') {
		    j++;
	       } else if (ch == '\n') {
		    check(((j+1) == n), "Error in the number of elements readed")
		    i++;
		    j = 0;
	       } else {
		    log_err("unknown character");
		    exit(1);
	       }
	       string_clean(&buf);
	  }
	  ch = getc(f);
     }
     fclose(f);
     free_array(buf);
     return m_avg/k;
}

static void load_cluster_data(const struct hash_map *interact, struct cluster_array *ca)
{
     FILE *f;
     char buf[BUFSZ];
     unsigned int i, n;
     char *d, *t, *key;
     int l, cont;
     struct item_entry *item;
     struct hash_entry *hentry;
     bool added;
     
     n = ca->nr;
     for (i = 0; i < n; i++) {
	  f = fopen(ca->data[i].fname, "r");
	  check(f, "Error, no instance file specified, abort");
	  cont = 0;
	  while (fgets(buf, sizeof(buf), f) != NULL) {
	       d = strtok(buf, "\t");
	       t = strtok(NULL, "\t");
	       l = asprintf(&key,"%s%s%s", d, sep, t);
	       check((l != -1), "Error making a key");
	       hentry = hmap_find_member(interact, key, l);
	       if (hentry == NULL) {
		    fprintf(stderr, "Error: interaction %s not founded", key);
		    exit(1);
	       }
	       item = hash_entry(hentry, struct item_entry, entry);
	       added = insert_hash_iset(&ca->data[i].vertices, item->id);
	       check(added, "Error, repetead edge");
	       free(key);
	       cont++;
	  }
	  if (feof(f)){
	       check((cont > 0), "Error, cluster with zero elements");
	       fclose(f);
	  } else {
	       fprintf(stderr, "Error reading file: %s", ca->data[i].fname);
	       exit(1);
	  }
     }
}

/**************************
* Conductance computation
***************************/

static void compute_weight_degree(const struct square_matrix *simM,
				  struct double_array *wdeg)
{
     unsigned int i, j, n;

     n = simM->n;
     wdeg->data = (double *)xcalloc(n, sizeof(double));
     wdeg->alloc = n;
     wdeg->nr = n;
     for (i = 0; i < n; i++) {
	  wdeg->data[i] = 0.0;
	  for (j = 0; j < n; j++) {
	       if (i != j) /* We not use loops */
		    wdeg->data[i] += simM->data[i][j];
	  }
     }
}

static inline  double compute_vol(const struct int_array *s, const struct double_array *wdeg)
{
     unsigned int i, n;
     double vol;

     vol = 0.0;
     n = s->nr;
     for (i = 0; i < n; i++) 
	  vol += wdeg->data[s->data[i]];
     return vol;
}

static struct int_array set_complement(const struct hash_iset *set, unsigned int nVec)
{
     unsigned int i;
     struct int_array sc;
     bool inset;

     init_struct_array(sc);
     for (i = 0; i < nVec; i++) {
	  inset = lookup_hash_iset(set, i);
	  if (!inset)
	       ARRAY_PUSH(sc, i);
     }
     return sc;
}

static double weight_intra_cluster(const struct int_array *s, const struct int_array *sc,
				   const struct square_matrix *M)
{
     unsigned i, j, ns, nsc;
     double v;

     v = 0.0;
     ns = s->nr;
     nsc = sc->nr;
     for (i = 0; i < ns; i++) 
	  for (j = 0; j < nsc; j++) 
	       v += M->data[s->data[i]][sc->data[j]];
     return v;
}

static double compute_cluster_conductance(struct cluster *c,
					  const struct square_matrix *M,
					  const struct double_array *wdeg)
{
     double v, wic, vol_s, vol_sc;
     struct int_array s, sc;
     
     if ( (c->vertices.nr == M->n) || (c->vertices.nr ==  0) )  {
	  v =  1.0;
#ifdef PRGDEBUG
	  printf("- Case 1 - Conductance %.5f\n", v);
#endif
     } else {
     	  sc = set_complement(&c->vertices, M->n);
	  s = iset_to_array(&c->vertices);
	  wic = weight_intra_cluster(&s, &sc, M);
	  if (wic < EPSILON) {
	       free_array(sc);
	       free_array(s);
	       v = 0.0;
#ifdef PRGDEBUG
	       printf("-- Case 2 - Conductance %.5f\n", v);
#endif
	  } else {
	       vol_s = compute_vol(&s, wdeg);
	       check((vol_s > EPSILON), "Error the vol_s value is zero");
	       c->degree = vol_s; 
	       vol_sc = compute_vol(&sc, wdeg);
	       check((vol_sc > EPSILON), "Error the vol_sc value is zero");
	       v = wic / MIN(vol_s, vol_sc);
#ifdef PRGDEBUG
	       printf("--- Case 3 - Conductance %.5f\n", v);
#endif
	       free_array(sc);
	       free_array(s);
	  }
     }
     return v;
}

static void compute_all_cluster_conductance(struct cluster_array *ca,
					    const struct square_matrix *M,
					    const struct double_array *wdeg)
{
     unsigned int i, n;
     
     n = ca->nr;
     if (n == 1) 
	  check((ca->data[0].vertices.nr == M->n) , "Error, incomplete edges in the clustering");
     for (i = 0; i < n; i++)
	  ca->data[i].conductance = compute_cluster_conductance(&ca->data[i], M, wdeg);
}

static void print_clustering_metrics(struct cluster_array *ca,
				     double c_conductance_max, double c_conductance_min,
				     double c_conductance_avg, double c_coverage,
				     double c_modularity, double c_tcut, double perf)
{
     unsigned int i, n;
          
     printf("\n************************************\n");
     printf("Clustering measures");
     printf("\n************************************\n");
     printf("ClusterFilename\tConductance\n");
     n = ca->nr;
     for (i = 0; i < n; i++) 
	  printf("%s\t%.12f\n", ca->data[i].name, ca->data[i].conductance);
     printf("************************************\n");
     printf("Max conductance:\t%.12f\n", c_conductance_max);
     printf("Min conductance:\t%.12f\n", c_conductance_min);
     printf("Average conductance:\t%.12f\n", c_conductance_avg);
     printf("Coverage:\t%.12f\n", c_coverage);
     printf("Modularity:\t%.12f\n", c_modularity);
     printf("Total cut:\t%.12f\n", c_tcut);
     printf("Performance:\t%.12f\n", perf);
     printf("************************************\n");
}

/************************
* Coverage computation
*************************/

static double compute_clustering_weight(struct cluster_array *ca,
					const struct square_matrix *M)
{
     unsigned i, j, n, m, k, p;
     double t;
     struct int_array e;
	  
     t = 0.0;
     p = ca->nr;
     for (k = 0; k < p; k++) {
	  e = iset_to_array(&ca->data[k].vertices);
	  n = e.nr;
	  m = n - 1;
	  for (i = 0; i < m; i++) {
	       for (j = i+1; j < n; j++) {
		    t += M->data[e.data[i]][e.data[j]];
	       }
	  }
	  free_array(e);
     }
     return t;
}
     
static double compute_total_weight(const struct square_matrix *M)
{
     unsigned int i, j, n, m;
     double tw;
     
     n = M->n;
     m = n - 1;
     tw = 0.0;
     for (i = 0; i < m; i++) {
	  for (j = i+1; j < n; j++) {
	       tw += M->data[i][j]; 
	  }
     }
     return tw;
}

static inline double compute_coverage(double w, double wt)
{
     check((wt > EPSILON), "Error, total weight is zero");
     return w/wt;
}

/************************
* Modularity computation
*************************/

static double compute_modularity(const struct cluster_array *ca, double wc, double wt)
{
     double p, q, v;
     unsigned i, n;
     
     p = wc/wt;
     q = 0.0;
     n = ca->nr;
     for (i = 0; i < n; i++) {
	  v = ca->data[i].degree/(2 * wt);
	  q += v*v;
     }
     return p-q;
}

/************************
* Total cut computation
*************************/

static double total_cut(const struct cluster_array *clustering, const struct square_matrix *M)
{
     unsigned i, j, k, l, n, m;
     struct int_array c1, c2;
     double cut;

     cut = 0.0;
     n = clustering->nr;
     m = n - 1;
     for (i = 0; i < m; i++) {
	  c1 = iset_to_array( &clustering->data[i].vertices );
	  for (j = i+1; j < n; j++) {
	       c2 = iset_to_array( &clustering->data[j].vertices );
	       for (k = 0; k < c1.nr; k++) {
		    for (l = 0; l < c2.nr; l++) {
			 cut += M->data[ c1.data[k] ][ c2.data[l] ];
		    }
	       }
	       free_array(c2);
	  }
	  free_array(c1);
     }
     return cut;
}

/**************************
* Performance computation
**************************/

static double f_intra(struct cluster_array *ca, const struct square_matrix *M,
		      double m_weight, double theta)
{
     unsigned i, j, n, m, k, p;
     double t, factor1, factor2;
     struct int_array e;
	  
     t = 0.0;
     p = ca->nr;
     for (k = 0; k < p; k++) {
	  e = iset_to_array(&ca->data[k].vertices);
	  n = e.nr;
	  m = n - 1;
	  factor1 = m_weight*n*m;
	  factor2 = 0.0;
	  for (i = 0; i < m; i++) 
	       for (j = i+1; j < n; j++) 
		    factor2 += M->data[e.data[i]][e.data[j]];
	  factor2 = factor2*theta;
	  t += factor1 - factor2;
	  free_array(e);
     }
     return t;
}

static double g_inter(double wc, double wt)
{
     return (wt - wc);
}

static double performace(double f, double g, int n, double m_weight)
{
     return 1 - ( f+g )/( n*(n-1)*m_weight );
}

/************************
*        Main
*************************/

int main(int argc, char **argv)
{
     struct cluster_array cinput;
     clock_t ti, tf;
     struct hash_map interactions;
     struct square_matrix simM;
     unsigned int ncol;
     struct double_array wdeg;
     double wt, wc, coverage, conductance_avg, conductance_max, conductance_min,
	  modularity, tcut, f, g, perf, m_weight, theta, v; 

     ti = clock();
     if (argc == 4) {
	  m_weight = 1.0; /* Initialization to avoid the compiler warning */
	  theta = THETA;   
     } else if (argc == 6) {
	  m_weight = strtod(argv[4], (char **)NULL);
	  theta = strtod(argv[5], (char **)NULL); 
     } else {
	  printf("Error in the number of arguments:\n\tcma <cluster_directory> <graph_file> <matrix_file> [maximum_weight_M] [intra_weight]\n");
	  exit(1);
     }

     /*******************
      * Loading data
      */
     printf("\nStarting the application\n");
     printf("Cluster files folder: %s\n", argv[1]);
     cinput = get_all_files(argv[1]);
     printf("Number of cluster: %d\n", cinput.nr);
     load_interactions(&interactions, argv[2]);
     ncol = interactions.fill;
     printf("Number of edges: %u\n", ncol);
     simM.data = double_matrix(0, ncol-1, 0, ncol-1);
     simM.n = ncol;
     v = load_matrix(argv[3], &simM);
     printf("Average value of the matrix %.3f\n", v);
     if (argc == 4)
	  m_weight = v; /* We use the average value of the matrix */
     printf("Weight M %.3f\n", m_weight);
     printf("Intra edge weight theta %.3f\n", theta);
     load_cluster_data(&interactions, &cinput);
     printf("Similarity matrix loaded!\n");
#ifdef PRGDEBUG
     print_sim_matrix(&simM);
     print_cluter_array(&cinput);
#endif     
     printf("Computing measures .......\n");
     
     /**************************
      * Computing measures
      */
     init_struct_array(wdeg);
     compute_weight_degree(&simM, &wdeg);
#ifdef PRGDEBUG
     printf("Adjacent degrees:\n");
     print_double_array(&wdeg);
#endif
     compute_all_cluster_conductance(&cinput, &simM, &wdeg);
     conductance_max = max_conductance(&cinput);
     conductance_min = min_conductance(&cinput);
     conductance_avg = average_conductance(&cinput);
     wt = compute_total_weight(&simM);
     wc = compute_clustering_weight(&cinput, &simM);
     coverage = compute_coverage(wc, wt);
     check((coverage < ONE), "Error, parcial weight is greater than total weight");
     modularity = compute_modularity(&cinput, wc, wt);
     tcut = total_cut(&cinput, &simM); 
     f = f_intra(&cinput, &simM, m_weight, theta);
     g = g_inter(wc, wt);
     perf = performace(f, g, ncol, m_weight);
     print_clustering_metrics(&cinput, conductance_max, conductance_min,
			      conductance_avg, coverage, modularity, tcut, perf);
     
      /**************************
      * Freeing memory
      */
     free_cluster_array(&cinput);
     free_hash_map_item(&interactions);
     free_double_matrix(simM.data, 0, 0);
     free_array(wdeg);

     tf = clock();
     printf("\nTotal time %.3f secs\n", (double)(tf-ti)/CLOCKS_PER_SEC);
     return 0;
}
