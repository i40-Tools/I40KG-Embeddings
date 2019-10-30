/**
 * Copyright (C) 2012-2015 Universidad Simón Bolívar
 *
 * Copying: GNU GENERAL PUBLIC LICENSE Version 2
 * @author Guillermo Palma <gvpalma@usb.ve>
 */

#ifndef ___TYPES_H
#define ___TYPES_H

struct int_array {
     unsigned int nr;
     unsigned int alloc;
     int *data;
};

struct matrix {
     int start;
     int end;
     double **data;
};

struct string_array {
     unsigned nr;
     unsigned alloc;
     char **data;
};

#endif /* ___TYPES_H */
