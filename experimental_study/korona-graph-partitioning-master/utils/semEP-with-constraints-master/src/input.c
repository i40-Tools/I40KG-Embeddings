/**
 * Copyright (C) 2013-2015 Universidad Simón Bolívar
 *
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gvpalma@usb.ve>
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "types.h"
#include "memory.h"
#include "hash_map.h"
#include "util.h"
#include "input.h"

#define BUFSZ     256

struct concept {
     int pos;
     struct hash_entry entry;
};

struct char_array {
     unsigned nr;
     unsigned alloc;
     char *data;
};

struct edge {
     char *left;
     char *right;
     char *relation;
     double weight;
};

struct graph_data {
     unsigned nr;
     struct edge *edge_array;
};

/*************************************
 **  Utilities
 ************************************/

#ifdef PRGDEBUG
static void print_int_array(struct int_array *v)
{
  printf("****************\n");
  for (unsigned i = 0; i < v->nr; i++) {
    printf("%d ", v->data[i]);
  }
  printf("\n");
}

static void print_node_prt_array(struct node_ptr_array *a)
{
     printf("\nNumber of elements: %u\n", a->nr);
     for (unsigned i = 0; i < a->nr; i++) {
	  printf("id %d pos left %d pos right %d sim %.3f\n",
		 a->data[i]->id, a->data[i]->pos1, a->data[i]->pos2, a->data[i]->sim);
     }
}
#endif


/*********************************
 ** String processing
 *********************************/

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

static inline void init_char_array(struct char_array *buf)
{
     buf->data = xcalloc(BUFSZ, 1);
     buf->alloc = BUFSZ;
     buf->nr = 0;
}

static inline void string_clean(struct char_array *buf)
{
     buf->nr = 0;
}

static void free_string_array(struct string_array *sa)
{
     for (unsigned i = 0; i < sa->nr; i++)
	  if (sa->data[i])
	       free(sa->data[i]);
     free(sa->data);
     sa->nr = 0;
}

/*********************************
 ** Terms processing
 *********************************/

static void terms_load(struct string_array *sa, const char *terms_filename)
{
     FILE *f;
     char buf[BUFSZ];
     size_t last;
     unsigned i, n;
     int l;
     
     f = fopen(terms_filename, "r");
     if (!f) {
	  fatal("no terms file specified, abort\n");
     }
     if (fgets(buf, sizeof(buf), f) == NULL)
	  fatal("error reading file");
     errno = 0;
     n = strtol(buf, NULL, 10);
     if (errno)
	  fatal("error in the conversion of string to integer\n");
     ALLOC_GROW(sa->data, n+sa->nr, sa->alloc);
     for (i = 0; i < n; i++) {
	  if (fgets(buf, sizeof(buf), f) == NULL)
	       fatal("Error reading file");
	  last = strlen(buf) - 1;
	  if(buf[last] == '\n')
	       buf[last] = 0;
	  l = asprintf(&sa->data[sa->nr],"%s", buf);
	  if (l == -1)
	       fatal("error in term creation");
	  sa->nr++;
     }
     fclose(f);
}

#ifdef PRGDEBUG
static void print_string_array(struct string_array *sa)
{
     printf("\nNumber of elements %d\n", sa->nr);
     for (unsigned i = 0; i < sa->nr; i++)
	  printf("%s\n", sa->data[i]);
     printf("\n");
}
#endif

/*********************************
 ** Matrix processing
 *********************************/

#ifdef PRGDEBUG
static void print_sim_matrix(struct matrix *M)
{
     int i, j;
     
     printf("***********************\n");
     for (i = M->start; i < M->end; i++) {
	  for (j = M->start; j < M->end; j++) {
	       printf("%.3f ", M->data[i][j]);
	  }
	  printf("\n");
     }
     printf("***********************\n");
}
#endif

static void similarity_matrix_load(const char *filename, int start, struct matrix *m)
{
     FILE *f;
     struct char_array buf;
     int n, i, j, end;
     int ch;

     f = fopen(filename, "r");
     if (!f) {
	  fatal("No instance file specified, abort\n");
     }
     n = 0;
     init_char_array(&buf);
     ch = getc(f);
     errno = 0;
     /* read number of nodes and arcs */
     while((ch != '\n') && (ch != EOF)) {
	  add_char(&buf, ch);
	  ch = getc(f);
     }
     if (ch != EOF) {
	  add_char(&buf, '\0');
	  n = strtol(buf.data, NULL, 10);
	  if (errno)
	       fatal("error in the conversion of string to integer\n");
     } else {
	  fatal("error reading the matrix data file\n");
     }
     string_clean(&buf);
     m->start = start;
     end = n+start;
     m->start = start;
     m->end = end;
     m->data = double_matrix(start, end, start, end);
     i = start;
     j = start;
     ch = getc(f);
     if (ch == EOF) {
	  fatal("error reading the matrix data file\n");
     }
     errno = 0;
     while (ch != EOF) {
	  if ((ch != ' ') && (ch != '\n')) {
	       add_char(&buf, ch);
	  } else {
	       add_char(&buf, '\0');
	       m->data[i][j] = strtod(buf.data, NULL);
	       if (errno)
		    fatal("error in the conversion of string to double\n");
	       if (ch == ' ') {
		    j++;
	       } else if (ch == '\n') {
		    i++;
		    j = start;
	       } else {
		    fatal("unknown character");
	       }
	       string_clean(&buf);
	  }
	  ch = getc(f);
     }
     fclose(f);
     free_array(buf);
}

/*********************************************
 ** Mapping terms to positions of the array
 *********************************************/

#ifdef PRGDEBUG
static void print_hash_term(struct hash_map *term_pos)
{
     struct concept *item;
     struct hash_entry *hentry;

     printf("\nMap term-position\n");
     printf("Number of terms: %u\n", term_pos->fill);
     hmap_for_each(hentry, term_pos) {
	  item = hash_entry(hentry, struct concept, entry);
	  printf("** %s %d\n", hentry->key, item->pos);
     }
}
#endif

static void free_map_term_pos(struct hash_map *term_pos)
{
     struct concept *item;
     struct hash_entry *hentry;
     struct hlist_node *n;

     hmap_for_each_safe(hentry, n, term_pos) {
	  item = hash_entry(hentry, struct concept, entry);
	  hmap_delete(term_pos, hentry);
	  free(item);
     }
     hmap_destroy(term_pos);
}

static void map_term_pos(struct hash_map *term_pos, const struct string_array *td)
{
     int i, n;
     struct concept *item;
     size_t len;

     n = td->nr;
     hmap_create(term_pos, n*2+1);
     for (i = 0; i < n; i++) {
	  if (td->data[i] == NULL)
	       fatal("invalid name");
	  len = strlen(td->data[i]);
	  item = xmalloc(sizeof(struct concept));
	  item->pos = i;
	  if (hmap_add_if_not_member(term_pos, &item->entry, td->data[i], len) != NULL)
	       fatal("error, term %s is repeated in the file\n", td->data[i]);
     }
}

/*********************************
 ** Bipartite graph processing
 *********************************/

#ifdef PRGDEBUG
static void print_graph_data(const struct graph_data *td)
{
     printf("\nNumber of edges %d\n", td->nr);
     for (unsigned i = 0; i < td->nr; i++) {
	  printf("%s\t%s\t%.3f\t%s\n", td->edge_array[i].left, td->edge_array[i].right,
		 td->edge_array[i].weight, td->edge_array[i].relation);
     }
}
#endif

static void free_term_data(struct graph_data *td)
{
     for (unsigned i = 0; i < td->nr; i++) {
	  free(td->edge_array[i].right);
	  free(td->edge_array[i].left);
	  free(td->edge_array[i].relation);
     }
     td->nr = 0;
     free(td->edge_array);
}

static void graph_load(struct graph_data *td, const char *filename)
{
     FILE *f;
     struct char_array buf;
     int n, i;
     int ch, tok;
     size_t len;

     f = fopen(filename, "r");
     if (!f) {
	  fatal("no graph file specified, abort\n");
     }
     n = 0;
     init_char_array(&buf);
     ch = getc(f);
     errno = 0;
     /* read number of terms */
     while((ch != '\n') && (ch != EOF)) {
	  add_char(&buf, ch);
	  ch = getc(f);
     }
     if (ch != EOF) {
	  add_char(&buf, '\0');
	  n = strtol(buf.data, NULL, 10);
	  if (errno)
	       fatal("Error in the conversion of string to integer\n");
     } else {
	  fatal("Error reading the description data file\n");
     }
     string_clean(&buf);

     td->nr = n;
     td->edge_array = (struct edge *)xcalloc(n, sizeof(struct edge));
     /* read the terms */
     ch = getc(f);
     if (ch == EOF) {
	  fatal("Error reading the graph data file\n");
     }
     tok = 1;
     i = 0;
     while ((ch != EOF) && (i < n)) {
	  if ((ch != '\t') && (ch != '\n')) {
	       add_char(&buf, ch);
	  } else {
	       add_char(&buf, '\0');
	       if (ch == '\t') {
		    len = strlen(buf.data) + 1;
		    assert(len > 1);
		    if (tok == 1) {
			 td->edge_array[i].left = xmalloc(len);
			 strcpy(td->edge_array[i].left, buf.data);
		    } else if (tok == 2) {
			 td->edge_array[i].right = xmalloc(len);
			 strcpy(td->edge_array[i].right, buf.data);
		    } else if (tok == 3) {
			 td->edge_array[i].relation = xmalloc(len);
			 strcpy(td->edge_array[i].relation, buf.data);
		    } else {
			 fatal("error in graph file format\n");
		    }
		    tok++;
	       } else {
		    assert(ch == '\n');
		    if (tok != 4)
			 fatal("Error in term file format\n");
		    errno = 0;
		    td->edge_array[i].weight = strtod(buf.data, NULL);
		    if (errno)
			 fatal("error in the conversion of string to integer\n");
		    i++;
		    tok = 1;
	       }
	       string_clean(&buf);
	  }
	  ch = getc(f);
     }
     fclose(f);
     free_array(buf);
}

static void build_bipartite_graph(const struct hash_map *term_pos, const struct graph_data *gd,
				  struct node_ptr_array *bpgraph, struct int_array *lt,
				  struct int_array *rt, int n_left)
{
     unsigned i, n;
     int lp, rp;
     struct concept *item;
     struct hash_entry *hentry;
     struct node *new;
     bool *in_graph;
     int *pos_left, *pos_right;
     size_t len;
     
     n = term_pos->fill;
     in_graph = (bool *)xcalloc(n, sizeof(bool));
     memset(in_graph, false, n*sizeof(bool));
     
     pos_left = (int *)xcalloc(n, sizeof(int));
     pos_right = (int *)xcalloc(n, sizeof(int));
     
     for (i = 0; i < gd->nr; i++) {
	  len = strlen(gd->edge_array[i].left);
	  hentry = hmap_find_member(term_pos, gd->edge_array[i].left, len); 
	  if (hentry == NULL)
	       fatal("the node %s is not in the left elements\n", gd->edge_array[i].left);
	  item = hash_entry(hentry, struct concept, entry);
	  if (item == NULL)
	       fatal("Invalid element");
	  lp = item->pos;
	  if (lp >= n_left)
	       fatal("%s should be not in the left side", gd->edge_array[i].left);
	       	 	  
	  len = strlen(gd->edge_array[i].right);
	  hentry = hmap_find_member(term_pos, gd->edge_array[i].right, len); 
	  if (hentry == NULL)
	       fatal("the node %s is not in the right elements\n", gd->edge_array[i].right);
	  item = hash_entry(hentry, struct concept, entry);
	  if (item == NULL)
	       fatal("invalid element");
	  rp = item->pos;
	  if (rp < n_left)
	       fatal("%s should be not in the left side", gd->edge_array[i].right);
	  
	  if (!in_graph[lp]) {
	       in_graph[lp] = true;
	       pos_left[lp] = lt->nr;
	       ARRAY_PUSH(*lt, lp);
	  }

	  if (!in_graph[rp]) {
	       in_graph[rp] = true;
	       pos_right[rp] = rt->nr;
	       ARRAY_PUSH(*rt, rp);
	  }

	  new = xcalloc(1, sizeof(struct node));
	  new->id = i;
	  new->pos1 = pos_left[lp];
	  new->pos2 = pos_right[rp];
	  new->sim = gd->edge_array[i].weight;
	  len = strlen(gd->edge_array[i].relation) + 1;
	  new->relation = xmalloc(len);
	  strcpy(new->relation, gd->edge_array[i].relation);
	  new->cp = NULL;
	  ARRAY_PUSH(*bpgraph, new);
     }
     free(in_graph);
     free(pos_right);
     free(pos_left);
}

/*********************************
 ** Input Data
 *********************************/

void free_input_data(struct input_data *in)
{
     free_double_matrix(in->left_matrix.data, in->left_matrix.start, in->left_matrix.start);
     free_double_matrix(in->right_matrix.data, in->right_matrix.start, in->right_matrix.start);
     free_array(in->left_terms);
     free_array(in->right_terms);
     free_string_array(&in->td);
     for (unsigned i = 0; i < in->bpgraph.nr; i++)
	  if (in->bpgraph.data[i])
	       free(in->bpgraph.data[i]);
     free(in->bpgraph.data);
}

struct input_data get_input_data(const char *matrix_left_filename, const char *matrix_right_filename,
				 const char *left_filename, const char *right_filename,
				 const char *graph_filename)
{
     int nl;
     struct input_data in;
     struct hash_map term_pos;
     struct graph_data gd;

     init_struct_array(in.td);
     terms_load(&in.td, left_filename);
     nl = in.td.nr;
     terms_load(&in.td, right_filename);
     similarity_matrix_load(matrix_left_filename, 0, &in.left_matrix);
     similarity_matrix_load(matrix_right_filename, nl, &in.right_matrix);
     printf("Left matrix has %d elements\n", (in.left_matrix.end-in.left_matrix.start));
     printf("Right matrix has %d elements\n",(in.right_matrix.end-in.right_matrix.start));
     map_term_pos(&term_pos, &in.td);
     init_struct_array(in.bpgraph);
     graph_load(&gd, graph_filename);
     init_struct_array(in.left_terms);
     init_struct_array(in.right_terms);
     init_struct_array(in.bpgraph);
     build_bipartite_graph(&term_pos, &gd, &in.bpgraph, &in.left_terms, &in.right_terms, nl);
#ifdef PRGDEBUG
     print_string_array(&in.td);
     print_sim_matrix(&in.left_matrix);
     print_sim_matrix(&in.right_matrix);
     print_hash_term(&term_pos);
     print_graph_data(&gd);
     print_int_array(&in.left_terms);
     print_int_array(&in.right_terms);
     print_node_prt_array(&in.bpgraph);
#endif
     free_map_term_pos(&term_pos);
     free_term_data(&gd);
     
     return in;
}
