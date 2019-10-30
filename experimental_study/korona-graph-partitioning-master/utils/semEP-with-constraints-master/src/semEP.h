/**
 * Copyright (C) 2013-2015 Universidad Simón Bolívar
 *
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gvpalma@usb.ve>
 */

#ifndef __SEMEP_H
#define __SEMEP_H

#include "types.h"
#include "hash_iset.h"

struct color {
     int id;
     double sim_entity_1;
     double sim_entity_2;
     double sim_between;
     double cDensity;
     struct int_array id_nodes;
     struct hash_iset entities1;
     struct hash_iset entities2;
};

struct node {
     int id;
     int pos1;
     int pos2;
     double sim;
     char *relation;
     struct color *cp;
};

struct node_ptr_array{
     unsigned nr;
     unsigned alloc;
     struct node **data;
};

double semEP_solver(const struct matrix *lmatrix, const struct matrix *rmatrix,
		    const struct int_array *lterms, const struct int_array *rterms,
		    const struct string_array *desc, struct node_ptr_array *color_nodes,
		    double lthreshold, double rthreshold,
		    const char *bpgraph_name, bool rel_constraint, bool prediction);

#endif /* __SEMEP_H */
