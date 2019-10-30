/**
 * Copyright (C) 2015 Universidad Simón Bolívar
 * Copyright (C) 2017 Forschungszentrum L3S
 *
 * @brief Implementation of a adjacent list graph
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gvpalma@usb.ve>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include "util.h"
#include "memory.h"
#include "graph_adj.h"

void init_graph_adj(struct graph_adj *g, int nodes)
{
     g->n_nodes = nodes;
     g->n_arcs = 0;
     g->adj_list = (struct arc_array *)xcalloc(nodes, sizeof(struct arc_array));
     g->degree = (struct adj *)xcalloc(nodes, sizeof(struct adj));
     for (int i = 0; i < nodes; i++) {
	  g->degree[i].din = 0;
	  g->degree[i].dout = 0;
	  init_struct_array( g->adj_list[i] );
     }
}

void add_arc(struct graph_adj *g, int id, int from, int to)
{
     struct arc a;

     a.id = id;
     a.to = to;
     ARRAY_PUSH(g->adj_list[from], a);
     g->degree[from].dout++;
     g->degree[to].din++;
     g->n_arcs++;
}

void print_graph_adj(struct graph_adj *g)
{
     struct arc current;
     
     printf("\nNum. of Nodes %d --- Num. of Arcs %ld\n",g->n_nodes, g->n_arcs);
     for (int i = 0; i < g->n_nodes; i++) {
	  printf("Node %d - (in out) (%d, %d): ", i,  g->degree[i].din, g->degree[i].dout);
	  for (unsigned int j = 0; j < g->adj_list[i].nr; j++) {
	       current = g->adj_list[i].data[j];
	       printf("(id %d f %d t %d) ",  current.id, i, current.to);
	  }
	  printf("\n");
     }
}

void free_graph_adj(struct graph_adj *g)
{    
     free(g->degree);
     for (int i = 0; i < g->n_nodes; i++) {
	  free_array( g->adj_list[i] );
     }
     free(g->adj_list);
     g->n_nodes = 0;
     g->n_arcs = 0;
}

struct arc_array get_adjacent_list(const struct graph_adj *g, int from)
{
     return g->adj_list[from];
}
