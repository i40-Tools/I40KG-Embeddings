/**
 * Copyright (C) 2015 Universidad Simón Bolívar
 * Copyright (C) 2017 Forschungszentrum L3S
 *
 * @brief Implementation of a adjacent list graph
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gvpalma@usb.ve>
 */

#ifndef ___GRAPH_ADJ_H
#define ___GRAPH_ADJ_H

struct arc {
     int id;
     int to;
};

struct arc_array {
     unsigned int nr;
     unsigned int alloc;
     struct arc *data;
};

struct graph_adj {
     int n_nodes;
     long n_arcs;
     struct adj {
	  int din;
	  int dout;
     } *degree;
     struct arc_array *adj_list;
};

void init_graph_adj(struct graph_adj *g, int nodes);

void add_arc(struct graph_adj *g, int id, int from, int to);

void print_graph_adj(struct graph_adj *g);

struct arc_array get_adjacent_list(const struct graph_adj *g, int from);

void free_graph_adj(struct graph_adj *g);

#endif
