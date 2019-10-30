/**
 * Copyright (C) 2013-2016 Universidad Simón Bolívar
 * Copyright (C) 2017 Forschungszentrum L3S
 *
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gvpalma@usb.ve>
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> 
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <math.h>

#include "dlist.h"
#include "graph_adj.h"
#include "memory.h"
#include "hash_iset.h"
#include "hash_map.h"
#include "util.h"
#include "semEP.h"

#define NOCOLOR      -1
#define NONODE       -1
#define NOCLUSTER    -1
#define INFTY         INT_MAX
#define EPSILON       10e-7
#define MARK          200000000

#define CHECK_ELEM(e)					      \
     do {						      \
	  if ((e) == NOELEM)				      \
	       fatal("Error in searching a valid element\n"); \
     } while(0)

/*
 * Macros used for binaries heaps implementation
 */
#define PARENT(i)     ((int)(((i) - 1) / 2))
#define LEFT(i)       (((i) * 2) + 1)
#define RIGHT(i)      (((i) * 2) + 2)

/*
 * Structures
 */

typedef struct color_ptr_array {
     unsigned nr;
     unsigned alloc;
     struct color **data;
} color_ptr_array_t;

typedef struct item_entry {
     double value;
     struct hash_entry entry;
} item_entry_t;

typedef struct saturation {
     int node;
     int n_adj_colors;
     struct hash_iset color_used;
} saturation_t;

typedef struct pqueue {
     int size;
     int *node_pos;
     saturation_t **heap;
} pqueue_t; 

typedef struct prediction {
     int cluster;
     int entity1;
     int entity2;
     double prob;
} prediction_t;

typedef struct prediction_array {
     unsigned nr;
     unsigned alloc;
     prediction_t *data;
} prediction_array_t;

typedef struct clusters {
     double nc;                    /* Value to minimizing */
     color_ptr_array_t partitions; /* Array with the clustes */
} clusters_t;

typedef struct info_partition {
     bool is_new_color;
     int color;
     double nc;
     double sim_entity_1;
     double sim_entity_2;
     double sim_between;
     double cDensity;
} info_partition_t;

typedef struct sim_tuple {
     double total;
     double sim;
} sim_tuple_t;

/*
 * Variables to get the simimilarity
 */

static int sl, sr, el, er;
static double **ML;
static double **MR;

/*************************************
 *************************************
 **
 ** Similarity functions and utilities
 **
 ************************************
 ************************************/

/**
 *  Function to get the similirity between terms
 */
static inline double similarity(int a, int b)
{
     double sim = 0.0;
     
     if (((a >= sl) && (a < el)) && ((b >= sl) && (b < el))) {
#ifndef PRGDEBUG
	  sim = ML[a][b];
#else
	  /* If the matrix is not symmetric we use of max value */ 
	  sim = MAX(ML[a][b], ML[b][a]);
#endif
	  
     } else if ((a >= sr) && (a < er) && (b >= sr) && (b < er)) {
#ifndef PRGDEBUG
	  sim = MR[a][b];
#else
	  /* If the matrix is not symmetric we use of max value */ 
	  sim = MAX(MR[a][b], MR[b][a]);
#endif
     } else {
	  fatal("Error in computing the similarity between %d and %d\n", a, b);
     }
     return sim;
}

static void init_clusters(clusters_t *c)
{
     c->partitions.alloc = 0;
     c->partitions.nr = 0.0;
     c->partitions.data = NULL;
}

#ifdef PRGDEBUG
static void print_int_array(struct int_array *v)
{
     unsigned int i;
     
     printf("\n");
     for (i = 0; i < v->nr; i++) {
	  printf("%d ", v->data[i]);
     }
     printf("\n");
}

static inline bool eq_double(double x, double y)
{
     return (fabs(x - y) < EPSILON);
}

static void print_coloring(clusters_t *clusters)
{
     struct color *c;
     unsigned int i;
     
     printf("nc %.4f\n", clusters->nc);
     for (i = 0; i < clusters->partitions.nr; i++) {
	  c = clusters->partitions.data[i];
	  assert((unsigned)c->id == i);
	  printf("%d %.4f %.4f %.4f %u %u %u\n", 
		 c->id, c->sim_entity_1, c->sim_entity_2, c->sim_between, 
		 c->id_nodes.nr, c->entities1.nr, c->entities2.nr);
     }
}
#endif

/**
 *  Build the graph to coloring
 */
static void build_graph_to_coloring_matrix(struct graph_adj *gc,
					   struct node_ptr_array *vn, 
					   const struct int_array *v1,
					   const struct int_array *v2,
					   double threshold1, double threshold2,
					   bool rel_constr)
{
     int i, j, n, a, b, c, d, cont;
     struct node *x, *y;
     bool dissimilar;
     bool dif_relation;
     
     n = vn->nr;
     cont = 0;
     for (i = 0; i < n-1; i++) {
	  x = vn->data[i];
	  a = x->pos1;
	  b = x->pos2;
	  for (j = i+1; j < n; j++) {
	       y = vn->data[j];
	       c = y->pos1;
	       d = y->pos2;
	       dissimilar = (similarity(v1->data[a], v1->data[c]) <= threshold1) ||
		    (similarity(v2->data[b], v2->data[d]) <= threshold2); 
	       if (rel_constr) {
		    if (strcmp(vn->data[i]->relation, vn->data[j]->relation) == 0)
			 dif_relation = false;			 
		    else
			 dif_relation = true;
		    if (dif_relation ||  dissimilar) {
			 add_arc(gc, cont, i, j);
			 add_arc(gc, cont, j, i);
			 cont++;
		    }
	       } else {
		    if (dissimilar) {
			 add_arc(gc, cont, i, j);
			 add_arc(gc, cont, j, i);
			 cont++;
		    }
	       }
	  }
     }
}

static void free_color(struct color *c)
{
     free(c->id_nodes.data);
     free_hash_iset(&c->entities1);
     free_hash_iset(&c->entities2);
     free(c);
}

/*************************************
 *************************************
 **
 ** Checking and control
 **
 ************************************
 ************************************/

/**
 * Pairwise symilarity for elements of a same type
 */
static double pairwise_similarity(struct hash_iset *entities)
{
     double total_sim;
     unsigned int i, j, n, k, l, alloc;
     int a, b;
     int *data;
   
     data = entities->htable;
     n = entities->nr;
     alloc = entities->alloc;
     if (n == 0) {
	  total_sim = 0.0;
     } else if (n == 1) {
	  k = 0;
	  a = NOELEM;
	  for (k = 0; k < alloc; k++) {
	       if (data[k] != NOELEM) {
		    a = data[k];
		    break;
	       }
	  }
	  CHECK_ELEM(a);
	  total_sim = similarity(a, a);
     } else {
	  total_sim = 0.0;
	  k = 0;
	  i = 0;
	  while ((i < n-1) && (k < alloc)) {
	       if (data[k] != NOELEM) {
		    a = data[k];
		    j = i+1;
		    l = k+1; 
		    while ((j < n) && (l < alloc)) {
			 if (data[l] != NOELEM) {
			      b = data[l];
			      assert(a != b);
			      total_sim += similarity(a, b);
			      j++;
			 }
			 l++;
		    }
		    assert(j == n);
		    i++;
	       }
	       k++;
	  }
	  assert(i == (n-1)); 
     }
     return total_sim;
}

/**
 * Cluster density
 */
static double cDensity(struct color *c)
{
     double cdensity, s1, s2, bpe;
     unsigned int n;
     
     n = c->id_nodes.nr;
     if (n == 0) {
	  error("Warning, processing a empty partition\n");
	  if (c->entities1.nr != 0)
	       fatal("The set of entities 1 is not empty");
	  if (c->entities2.nr != 0)
	       fatal("The set of entities 2 is not empty");
	  cdensity = 0.0;
     } else if (n == 1){
	  if (c->entities1.nr != 1)
	       fatal("The set of entities 1 does not have a single element");
	  if (c->entities2.nr != 1)
	       fatal("The set of entities 2 does not have a single element");
	  cdensity = EPSILON;
     } else {
	  cdensity = 0.0;
	  n = c->entities1.nr;
	  assert(n > 0);
	  if (n > 1)
	       s1 = pairwise_similarity(&c->entities1) / ((n*n-n)/2.0);
	  else
	       s1 = pairwise_similarity(&c->entities1);
	  n = c->entities2.nr;
	  assert(n > 0);
	  if (n > 1)
	       s2 = pairwise_similarity(&c->entities2) / ((n*n-n)/2.0);
	  else
	       s2 = pairwise_similarity(&c->entities2);
	  bpe = (double)c->sim_between/c->id_nodes.nr;
	  cdensity = (bpe+s1+s2)/3.0;
     }
     return cdensity;
}

#ifdef PRGDEBUG
/**
 * Function to minimizing for the clustering
 */
static double color_density(const clusters_t *c)
{
     unsigned int i, n_colors;
     double nc;
     
     nc = 0.0;
     n_colors = c->partitions.nr;
     for (i = 0; i < n_colors; i++) {
	  nc += (i+1) - cDensity(c->partitions.data[i]);
     }
     return nc;
}
#endif

/*************************************
 *************************************
 **
 **  Priority Queue
 **
 ************************************
 ************************************/

static inline int compare_saturation(const struct graph_adj *g, const saturation_t *a,
				     const saturation_t *b)
{
     int r;

     r = 0;
     if (a->n_adj_colors > b->n_adj_colors) { 
	  r = 1;
     } else if (a->n_adj_colors < b->n_adj_colors) {
	  r = -1;
     } else {
	  if (g->degree[a->node].din > g->degree[b->node].din) {
	       r = 1;
	  } else if (g->degree[a->node].din < g->degree[b->node].din) { 
	       r = -1;
	  } else {
	       /* Numerical order */
	       if (a->node < b->node) {
		    r = 1;
	       } else if (a->node > b->node) {
		    r = -1;
	       } else {
		    fatal("Nodes to compare are the same");
	       }
	  }
     }
     return r;
}

static inline void free_saturation_node(saturation_t *node)
{
     if (node) {
	  free_hash_iset(&node->color_used);
	  free(node);
     }
}

static inline void pq_init(pqueue_t *pq)
{
     pq->size = 0;
     pq->heap = NULL;
     pq->node_pos = NULL;
}

static inline void pq_delete(pqueue_t *pq)
{
     int i;
     
     for(i = 0; i < pq->size; i++)
	  free_saturation_node(pq->heap[i]);
     free(pq->node_pos);
     free(pq->heap);
}

static inline void pq_insert(const struct graph_adj *g, pqueue_t *pq,
			     saturation_t *node)
{
     int i, p;
     saturation_t **tmp;

     tmp = xrealloc(pq->heap, (pq->size+1)*sizeof(saturation_t *));
     pq->heap = tmp;
     pq->heap[pq->size] = node;
     i = pq->size;
     p = PARENT(i);
     while((i > 0) &&  (compare_saturation(g, pq->heap[p], pq->heap[i]) < 0)){
	  SWAP(pq->heap[p], pq->heap[i]);
	  i = p;
	  p = PARENT(i);
     }
     pq->size++;
}

static int extract_max(const struct graph_adj *g, pqueue_t *pq, saturation_t **node)
{
     int i, j, l, r;
     saturation_t *aux;
     saturation_t **tmp;
          
     if(pq->size == 0)
	  return -1;

     *node = pq->heap[0];
     aux =  pq->heap[pq->size-1];
     SWAP(pq->node_pos[pq->heap[0]->node],
	  pq->node_pos[pq->heap[pq->size-1]->node]); /* SWAP the positions*/
     if((pq->size - 1) > 0){
	  tmp = (saturation_t **)xrealloc(pq->heap, (pq->size-1)*sizeof(saturation_t *));
	  pq->heap = tmp;
	  pq->size--;
     } else {
	  free(pq->heap);
	  pq->heap = NULL;
	  free(pq->node_pos);
	  pq->node_pos = NULL;
	  pq->size = 0;
	  return 0;
     }
     pq->heap[0] = aux;
     i = 0;
     while (true) {
	  l = LEFT(i);
	  r = RIGHT(i);
	  if((l < pq->size) && (compare_saturation(g, pq->heap[l], pq->heap[i]) > 0))
	       j = l;
	  else
	       j = i;

	  if((r < pq->size) && (compare_saturation(g, pq->heap[r], pq->heap[j]) > 0))
	       j = r;

	  if( j == i ) {
	       break;
	  } else {
	       SWAP(pq->node_pos[pq->heap[j]->node],
		    pq->node_pos[pq->heap[i]->node]); /* SWAP the positions*/
	       SWAP(pq->heap[j], pq->heap[i]);
	       i = j;
	  }
     }
     return 0;
}

static int increase_key(const struct graph_adj *g, pqueue_t *pq, int node, int color)
{
     int i, p, pos;
	  
     if (pq->size == 0)
	  return -1;

     pos = pq->node_pos[node];
     if (pos >= pq->size)
	  pos = -1;
   
     if (pos == -1)
	  return -2;

     if (insert_hash_iset(&(pq->heap[pos]->color_used), color))
	  pq->heap[pos]->n_adj_colors++;
     else
	  return 0;
     
     i = pos;
     p = PARENT(i);
     while((i > 0) && (compare_saturation(g, pq->heap[p], pq->heap[i]) < 0)){
	  SWAP(pq->node_pos[pq->heap[p]->node],
	       pq->node_pos[pq->heap[i]->node]); /* SWAP the positions*/
	  SWAP(pq->heap[p], pq->heap[i]);
	  i = p;
	  p = PARENT(i);
     }
     return 0;
}

static void init_saturation_pq(const struct graph_adj *g, pqueue_t *pq)
{
     int i, n;
     saturation_t *ns;
     
     n = g->n_nodes;
     for (i = 0; i < n; i++) {
	  ns = (saturation_t *)xmalloc(sizeof(saturation_t));
	  ns->node = i;
	  ns->n_adj_colors = 0;
	  init_hash_iset(&ns->color_used);
	  pq_insert(g, pq, ns);	  
     }
     assert(pq->size == n);
     pq->node_pos = (int *)xmalloc(n*sizeof(int));
     for (i = 0; i < n; i++) {
	  pq->node_pos[pq->heap[i]->node] = i;
     }
}

/*************************************
 *************************************
 **
 **  Coloration Solver
 **
 ************************************
 ************************************/

static inline void init_and_add(struct hash_iset *s, int e)
{
     init_hash_iset(s);
     insert_hash_iset(s, e);
}

static inline struct color *new_color(int id, double sim_bt, int node, int e1, int e2)
{
     struct color *new;
     struct int_array aux = {0, 0, NULL};
    
     new = xcalloc(1, sizeof(struct color));
     new->id = id;
     new->sim_between = sim_bt;
     new->id_nodes = aux;
     ARRAY_PUSH(new->id_nodes, node);
     init_and_add(&new->entities1, (unsigned int)e1);
     init_and_add(&new->entities2, (unsigned int)e2);
     new->sim_entity_1 = pairwise_similarity(&new->entities1);
     new->sim_entity_2 = pairwise_similarity(&new->entities2);

     return new;
}

static inline int get_color(const struct node_ptr_array *node_color, int pos)
{
     int color;
     struct node *node_ptr;

     if (pos < 0)
	  fatal("Invalid position");
     if (node_color == NULL)
	  fatal("Invalid pointer to color");
     node_ptr = node_color->data[pos];
     if (node_ptr->cp == NULL) {
	  color = NOCOLOR;
     } else {
	  color = node_ptr->cp->id;
     }
     return color;
}

static void update_saturation_degree(const struct graph_adj *g, pqueue_t *pq, int node,
				     const struct node_ptr_array *node_color)
{
     int r, color;
     struct arc_array adjs;
     unsigned int i, nr;
	  
     color = get_color(node_color, node);
     assert(color != NOCOLOR);
     adjs = get_adjacent_list(g, node);
     nr = adjs.nr;
     for (i = 0; i < nr; i++) {
	  r = increase_key(g, pq, adjs.data[i].to, color);
	  if (r == -1)
	       fatal("Error in update saturation degree\n");
     }
}

static int greatest_saturation_node(const struct graph_adj *g, pqueue_t *pq, 
				    const struct node_ptr_array *node_color)
{
     int r, color, node;
     saturation_t *ns;

     node = NONODE;
     ns = NULL;
     color = INFTY;
     r = extract_max(g, pq, &ns);

     if (r == -1)
	  fatal("No node without color");
     if (ns) {
	  color = get_color(node_color, ns->node);
	  node = ns->node;
     } else {
	  fatal("Error in get the greatest saturation node");
     }
     if (color != NOCOLOR)
	  fatal("Error in node to coloring");
#ifdef PRGDEBUG
     printf("Node %d; Num. of adjacent %d; Degree in %d\n", node, ns->n_adj_colors, g->degree[node].din);
#endif
     free_saturation_node(ns); 
     return node;
}

static bool *get_ady_used_color(const struct graph_adj *g,
				const struct node_ptr_array *node_color, int node)
{
     //struct arc *current;
     struct arc_array adjs;
     bool *color_used;
     int color;
     size_t alloc;
     unsigned int i, nr;
     
     alloc = g->n_nodes*sizeof(bool);
     color_used = xmalloc(alloc);
     memset(color_used, false, alloc);
     adjs = get_adjacent_list(g, node);
     nr = adjs.nr;
     for (i = 0; i < nr; i++) {
	  color = get_color(node_color, adjs.data[i].to);
	  assert(color < g->n_nodes);
	  if (color != NOCOLOR) 
	       color_used[color] = true;
     }
     return color_used;
}

static void get_free_colors(const struct graph_adj *g,
			    const struct node_ptr_array *solution,
			    int node, struct int_array *available_colors,
			    color_ptr_array_t *partitions)
{
     int cn;
     bool *color_used;
     struct color *ctmp;
     unsigned int i, n;
     
     n = partitions->nr;
     color_used = NULL;
     assert(g->n_nodes > node);
     assert(g->n_nodes >= (int)n+1); /* Number of colors not used is n+1 */
     color_used = get_ady_used_color(g, solution, node);
     cn = get_color(solution, node);
     if (cn != NOCOLOR) 
	  fatal("A adjacent node are using the same color");
     for (i = 0; i < n; i++) {
	  ctmp = partitions->data[i];
	  assert(ctmp->id == (int)i);
	  if (!color_used[i]) {
	       available_colors->data[available_colors->nr++] = i;
	  }
     }
     /* New color */
     if (available_colors->nr == 0)
	  available_colors->data[available_colors->nr++] = i;
     free(color_used);
}


static inline sim_tuple_t agregate_similarity(double current_sim,
					      const struct hash_iset *set, int e)
{
     unsigned int i, n, k, alloc;
     double sim, total;
     sim_tuple_t t;
     int *data;

     sim = 0.0;
     total = 0.0;
     n = set->nr;
     if (lookup_hash_iset(set, e)) {
	  total = current_sim;
	  if (n == 1) {
	       sim = current_sim;
	  } else { 
	       sim = current_sim / ((n*n-n)/2.0);  
	       
	  }
     } else {
	  data = set->htable;
	  k = 0;
	  i = 0;
	  alloc = set->alloc;
	  while ( (i < n) && (k < alloc)) {
	       if (data[k] != NOELEM) {
		    total += similarity(data[k], e);
		    i++;
	       }
	       k++;
	  }
	  assert(i == n);
	  /* If n == 1 the current_sim = similarity(a, a), then
	     the current_sim will be not added to  
	     partitions with n > 1 elements */  
	  if (n > 1)
	       total += current_sim;
	  n++;
	  sim = total / ((n*n-n)/2.0); 
     }
     t.total = total;
     t.sim = sim;
     return t;
}

static info_partition_t density_with_new_node(const color_ptr_array_t *partitions,
					      const struct node *new_node, const struct int_array *v1,
					      const struct int_array *v2, int color)
{
     struct color *cptr;
     double bt;
     info_partition_t ip;
     sim_tuple_t t1, t2;
     
     cptr = partitions->data[color];
     assert(cptr->id == color);
     t1 = agregate_similarity(cptr->sim_entity_1, &cptr->entities1, v1->data[new_node->pos1]);
     t2 = agregate_similarity(cptr->sim_entity_2, &cptr->entities2, v2->data[new_node->pos2]);
     ip.is_new_color = false;
     ip.color = color;
     ip.sim_entity_1 = t1.total; 
     ip.sim_entity_2 = t2.total;
     ip.sim_between = cptr->sim_between + new_node->sim;
     bt = (double)ip.sim_between/(cptr->id_nodes.nr + 1.0);
     ip.cDensity = (t1.sim + t2.sim + bt)/3.0;
     assert(ip.cDensity <= 1.0001);
     return ip;
}

static info_partition_t get_best_color(const struct node_ptr_array *vn, clusters_t *c,
				       int new_node, struct int_array *free_colors,
				       const struct int_array *v1, 
				       const struct int_array *v2)
{

     int i, n, n_colors, curr_color;
     double nc_best, nc_new, nc_current;
     struct node *nptr;
     info_partition_t ip_aux, ip_best;
     
     n = free_colors->nr;
     nc_best = INFTY;
     n_colors = c->partitions.nr;
     nc_current = c->nc;
     nptr = vn->data[new_node];
     for (i = 0; i < n; i++) {
	  curr_color = free_colors->data[i];
	  assert((curr_color >= 0) && (curr_color <= n_colors));
	  DEBUG("Color to evaluate %d ", curr_color);
	  if (n_colors == curr_color) {
	       /* We need to use a new color, then we have a partition with a element */
	       /* The density of a partition with a element is epsilon */
	       nc_new = nc_current + ((curr_color+1) - EPSILON);
	       ip_aux.is_new_color = true;
	       ip_aux.color = curr_color;
	       ip_aux.sim_between = nptr->sim;
	       ip_aux.cDensity = EPSILON;
	       ip_aux.nc = nc_new; 
	  } else {
	       /* We coloring the new node with a used color */
	       ip_aux = density_with_new_node(&c->partitions, nptr, v1, v2, curr_color);
	       nc_new = nc_current + c->partitions.data[curr_color]->cDensity - ip_aux.cDensity;
	       ip_aux.nc = nc_new;
	  }
	  if (nc_best > nc_new) {
	       nc_best = nc_new;
	       ip_best = ip_aux;
	  }
     }
     return ip_best;
}

static void set_colors(clusters_t *c, info_partition_t *ip, struct node *nptr,
		       const struct int_array *v1, const struct int_array *v2)
{
     int e1, e2;
     struct color *cptr = NULL;
     
     e1 = v1->data[nptr->pos1];
     e2 = v2->data[nptr->pos2];
     if (ip->is_new_color) {
	  cptr = new_color(ip->color, ip->sim_between, nptr->id, e1, e2);
	  cptr->cDensity = ip->cDensity;
	  ARRAY_PUSH(c->partitions, cptr);
     } else {
	  cptr = c->partitions.data[ip->color];
	  cptr->sim_entity_1 = ip->sim_entity_1;
	  cptr->sim_entity_2 = ip->sim_entity_2;
	  cptr->sim_between = ip->sim_between;
	  insert_hash_iset(&cptr->entities1, e1);
	  insert_hash_iset(&cptr->entities2, e2);
	  cptr->cDensity = ip->cDensity;
	  ARRAY_PUSH(cptr->id_nodes, nptr->id);
     }
     nptr->cp = cptr;
     c->nc = ip->nc;
}

/**
 * Partiion graph solver based on the coloring algorithm called DSATUR
 */ 
static void coloring(const struct graph_adj *g,
		     const struct node_ptr_array *nodes,
		     clusters_t *c,
		     const struct int_array *v1,
		     const struct int_array *v2)
{
     struct color *cptr;
     struct node *nptr;
     int colored_nodes, new_node, e1, e2;
     pqueue_t pq_saturation;
     struct int_array free_colors = {0, 0, NULL};
     info_partition_t ip;
     long n;
     
     colored_nodes = 0;
     ALLOC_GROW(free_colors.data, (unsigned int)g->n_nodes, free_colors.alloc);
     pq_init(&pq_saturation);
     init_saturation_pq(g, &pq_saturation);
     assert(c->partitions.nr == 0);

     /* We color a first node */
     new_node = greatest_saturation_node(g, &pq_saturation, nodes);
     if (new_node == NONODE)
	  fatal("Error getting the greatest saturation node");
     nptr = nodes->data[new_node];
     e1 = v1->data[nptr->pos1];
     e2 = v2->data[nptr->pos2];
     assert(new_node == nptr->id);
     cptr = new_color(0, nptr->sim, nptr->id, e1, e2);
     cptr->cDensity = EPSILON;
     ARRAY_PUSH(c->partitions, cptr);
     c->nc = 1.0 - cDensity(cptr); /* First color */
     /* assert(cDensity(cptr) == cDensity(c->partitions.data[c->partitions.nr-1])); */
     nptr->cp = cptr; 
     colored_nodes++;
     if (pq_saturation.size != 0)
	  update_saturation_degree(g, &pq_saturation, new_node, nodes);

     /* We color all the nodes */
     n = g->n_nodes;
     while (colored_nodes < n) {
	  new_node = greatest_saturation_node(g, &pq_saturation, nodes);
	  if (new_node == NONODE)
	       fatal("Error getting the greatest saturation node");
	  nptr = nodes->data[new_node];
	  assert(new_node == nptr->id);
	  free_colors.nr = 0;
	  get_free_colors(g, nodes, new_node, &free_colors, &c->partitions);
#ifdef PRGDEBUG
	  printf("Free colors:\n");
	  print_int_array(&free_colors);
#endif
	  ip = get_best_color(nodes, c, new_node, &free_colors, v1, v2);
	  set_colors(c, &ip, nptr, v1, v2);
	  colored_nodes++;
	  if (pq_saturation.size != 0)
	       update_saturation_degree(g, &pq_saturation, new_node, nodes);
     }
     if (pq_saturation.size != 0)
	  fatal("Incomplete coloration\n");
     pq_delete(&pq_saturation);
     free(free_colors.data);
}

static double get_density_average(clusters_t *c)
{
     unsigned i, n;
     double r;

     r = 0.0;
     n = c->partitions.nr;
     for (i = 0; i < n; i++) {
	  r += c->partitions.data[i]->cDensity;
#ifdef PRGDEBUG
	  double cdn = cDensity(c->partitions.data[i]);
	  if( !eq_double(c->partitions.data[i]->cDensity, cdn) )
	       fatal("\nDensity values are different %.7f %.7f\n", c->partitions.data[i]->cDensity, cdn);
#endif
     }
     return r/n;
}

/*************************************
 *************************************
 **
 **  Get output files
 **
 ************************************
 ************************************/

static char *print_output_files(struct node_ptr_array *color_nodes,
				struct color_ptr_array *partitions,
				const struct int_array *v1, const struct int_array *v2,
				double threshold_lf, double threshold_rg,
				const char *name, char **desc)
{
     FILE *f;
     unsigned i, j, n, m;
     char *output1, *output2, *message;
     struct stat st;
     struct node *edge;
     struct color *cluster;
     int id_node;
    
     if (asprintf(&output1, "%s-%.4f-%.4f-Clusters", name, threshold_lf, threshold_rg) == -1)
	  fatal("Error in output directory");
     if (stat(output1, &st) == -1)
	  mkdir(output1, 0700);
 
     if (asprintf(&message, "Cluster directory: %s\n", output1) == -1)
	  fatal("Error in output message");

     printf("Number of partitions: %u\n", partitions->nr);
     n = partitions->nr;
     for (i = 0; i < n; i++) {
	  cluster = partitions->data[i];
	  if (asprintf(&output2, "%s/%s-%u-%.4f-%.4f.txt",
		       output1, name, i, threshold_lf, threshold_rg) == -1)
	       fatal("Error in cluster file");
          f = fopen(output2, "w");
	  if (!f)
	       fatal("No descriptor file specified, abort\n");
	  m = cluster->id_nodes.nr;
	  for (j = 0; j < m; j++) {
	       id_node = cluster->id_nodes.data[j];
	       edge = color_nodes->data[id_node];
	       assert(cluster->id == edge->cp->id);
	       fprintf(f ,"%s\t%s\t%.4f\t%s\n", 
		       desc[v1->data[edge->pos1]],
		       desc[v2->data[edge->pos2]], edge->sim, edge->relation);
	  }
	  fclose(f);
	  free(output2);
     }
     free(output1);
     return message;
}

/*************************************
 *************************************
 **
 **  Get Predited Links
 **
 ************************************
 ************************************/

static void free_hash_map_item(struct hash_map *hmap)
{
     item_entry_t *item;
     struct hash_entry *hentry;
     struct hlist_node *n;

     hmap_for_each_safe(hentry, n, hmap) {
	  item = hash_entry(hentry, item_entry_t, entry);
	  hmap_delete(hmap, hentry);
	  free(item);
     }
     hmap_destroy(hmap);
}

static double get_cluster_probability(int nodes1, int nodes2, int n_edges)
{
     int n_nodes = nodes1 * nodes2;

     if (n_nodes == 0)
	  fatal("Zero Division");
     return (double)n_edges / n_nodes;
}

static void get_predicted_links(prediction_array_t *cluster_pred,
				struct node_ptr_array *color_nodes,
				struct color_ptr_array *partitions,
				const struct int_array *v1, const struct int_array *v2)
{
     unsigned int i, j, k, n_links, m, n, n_clusters, alloc1, alloc2, a1, a2;
     struct hash_map edges_obs;
     struct hash_entry *hentry;
     char *buf;
     item_entry_t *item;
     struct node *edge;
     struct color *cluster;
     int node1, node2, l;
     prediction_t ptemp;
     int n_edges[partitions->nr];
     double prob;
     int *array1, *array2;
     
     n_clusters = partitions->nr;
     n_links = color_nodes->nr;
     hmap_create(&edges_obs, 2*n_links+1);
     memset(n_edges, 0, partitions->nr*sizeof(int));     
     for (i = 0; i < n_links; i++) {
	  edge = color_nodes->data[i];
	  l = asprintf(&buf, "%d-%d-%d", edge->cp->id, v1->data[edge->pos1], v2->data[edge->pos2]);
	  if (l == -1)
	       fatal("Error in edge key creation");
	  item = (item_entry_t *)xmalloc(sizeof(item_entry_t));
	  assert((unsigned)edge->cp->id <= n_clusters); 
	  n_edges[edge->cp->id]++;
	  if (hmap_add_if_not_member(&edges_obs, &item->entry, buf, l) != NULL)
	       fatal("Error, repeat edge in bipartite graph\n");
	  free(buf);
     }
     for (i = 0; i < n_clusters; i++) {
	  cluster = partitions->data[i];
	  assert((unsigned)cluster->id == i);
	  n = cluster->entities1.nr;
	  m = cluster->entities2.nr;
	  array1 = cluster->entities1.htable;
	  array2 = cluster->entities2.htable;
	  prob = get_cluster_probability(n, m, n_edges[i]);
	  assert((prob >= 0.0) && (prob <= 1.0));
	  alloc1 = cluster->entities1.alloc;
	  alloc2 = cluster->entities2.alloc;
	  j = 0;
	  a1 = 0;
	  while ((j < n) && (a1 < alloc1)) {
	       if (array1[a1] != NOELEM) {
		    node1 = array1[a1];
		    k = 0;
		    a2 = 0;
		    while ((k < m) && (a2 < alloc2)) {
			 if (array2[a2] != NOELEM) {
			      node2 = array2[a2];
			      l = asprintf(&buf, "%d-%d-%d", i, node1, node2);
			      if (l == -1)
				   fatal("Error in made a new link");
			      hentry = hmap_find_member(&edges_obs, buf, l);
			      if (hentry == NULL) {
				   ptemp.cluster = i;
				   ptemp.entity1 = node1;
				   ptemp.entity2 = node2;
				   ptemp.prob = prob;
				   ARRAY_PUSH(*cluster_pred, ptemp);
			      }
			      free(buf);
			      k++;
			 }
			 a2++;
		    }
		    assert(k == m);
		    j++;
	       }
	       a1++;
	  }
	  assert(j == n);
     }
     free_hash_map_item(&edges_obs);
}

static void print_predicted_links(prediction_array_t *cluster_pred,
				  double threshold_lf, double threshold_rg,
				  const char *name, char **desc)
{
     unsigned i, n;
     prediction_t ptemp;
     FILE *f;
     char *output;
     int current;
     
     if (asprintf(&output, "%s-%.4f-%.4f-Predictions.txt", name, threshold_lf, threshold_rg) == -1)
	  fatal("Error in prediction file");
     f = fopen(output, "w");
     free(output);
     if (!f)
	  fatal("No descriptor file specified, abort\n");
     n = cluster_pred->nr;
     current = NOCLUSTER;
     for (i = 0; i < n; i++) {
	  ptemp = cluster_pred->data[i];
	  if (current != ptemp.cluster) {
	       current = ptemp.cluster;
	       fprintf(f, "Cluster\t%u\n", current);
	  }
	  fprintf(f, "%s\t%s\t%.4f\n", desc[ptemp.entity1], desc[ptemp.entity2], ptemp.prob);
     }
     printf("Number of links predicted: %d\n", n);
     fclose(f);
}

/*************************************
 *************************************
 **
 **  Main
 **
 ************************************
 ************************************/

double semEP_solver(const struct matrix *lmatrix,
		    const struct matrix *rmatrix,
		    const struct int_array *lterms,
		    const struct int_array *rterms,
		    const struct string_array *desc,
		    struct node_ptr_array *color_nodes,
		    double lthreshold, double rthreshold,
		    const char *bpgraph_name, bool rel_constraint, bool prediction)
{
     struct graph_adj gc;
     unsigned int i, n;
     clock_t ti, tf;
     char *message;
     double density;
     prediction_array_t cluster_pred = {0, 0, NULL};
     clusters_t c;
     
     init_clusters(&c);     
     density = 0.0;
     ML =  lmatrix->data;
     sl = lmatrix->start;
     el = lmatrix->end;
     MR =  rmatrix->data;
     sr = rmatrix->start;
     er = rmatrix->end;
     printf("Bipartite Graph - Num. of nodes left: %u; Num. of nodes right: %u; Num. of edges: %u\n", lterms->nr, rterms->nr, color_nodes->nr);
     ti = clock();
     init_graph_adj(&gc, color_nodes->nr);
     build_graph_to_coloring_matrix(&gc, color_nodes, lterms, rterms, lthreshold, rthreshold, rel_constraint);
     tf = clock();
     printf("Time to build the graph to coloring: %.4f secs\n", (double)(tf-ti)/CLOCKS_PER_SEC);
     printf("Graph to Coloring - Num. of Nodes: %d; Num. of Edges: %ld\n", gc.n_nodes, gc.n_arcs/2);
     ti = clock();
     if (gc.n_nodes != 0) {
	  coloring(&gc, color_nodes, &c, lterms, rterms);
	  density = get_density_average(&c);
#ifdef PRGDEBUG
	  if (!eq_double(color_density(&c), c.nc)) 
	       fatal("Error in the color density value\n");
	  print_coloring(&c);
#endif
     } else {
	  fatal("Graph to coloring has no nodes");
     }
     tf = clock();
     printf("Coloring solver time %.4f secs\n", (double)(tf-ti)/CLOCKS_PER_SEC);
     ti = clock();
     message = print_output_files(color_nodes, &c.partitions, lterms, rterms, 
				  lthreshold, rthreshold, bpgraph_name, desc->data);
     printf("%s", message);
     if (prediction) {
	  get_predicted_links(&cluster_pred, color_nodes, &c.partitions, lterms, rterms);
	  print_predicted_links(&cluster_pred, lthreshold, rthreshold, bpgraph_name, desc->data);
     }
     tf = clock();
     printf("Output writing time.: %.4f secs\n", (double)(tf-ti)/CLOCKS_PER_SEC);
     free_array(cluster_pred);
     free(message);
     n = c.partitions.nr;
     for (i = 0; i < n; i++)
	  free_color(c.partitions.data[i]);
     free(c.partitions.data);
     free_graph_adj(&gc);
     n = color_nodes->nr;
     for (i = 0; i < n; i++)
	  free(color_nodes->data[i]->relation);

     return density;
}
