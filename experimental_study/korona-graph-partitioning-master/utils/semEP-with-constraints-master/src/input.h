/**
 * Copyright (C) 2013-2015 Universidad Simón Bolívar
 *
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gvpalma@usb.ve>
 */

#ifndef ___INPUT_H
#define ___INPUT_H

#include "types.h"
#include "semEP.h"

struct input_data {
     struct matrix left_matrix;
     struct matrix right_matrix;
     struct int_array left_terms;
     struct int_array right_terms;
     struct string_array td;
     struct node_ptr_array bpgraph;
};

struct input_data get_input_data(const char *matrix_left_filename, const char *matrix_right_filename,
				 const char *left_filename, const char *right_filename, const char *graph_filename);

void free_input_data(struct input_data *in);

#endif /* ___INPUT_H */
