CMA
====

About
=====

Clustering Measures App (CMA). This application computes the following
quality measurements for clusterings: Conductance [2], Coverage [2],
Modularity [3][4], Total Cut [1], and Performace [2].

Because semEP performs clustering of relationships,  then to compute the
clustering measures, we will to use a similarity matrix of relationships.
There are many wayss to compute the similarity of a pair of relationships.
We recommend a method as follow. Given the  relations R1 = (a1, b1) and R2 = (a2, b2),
then we can compute the similarity(R1, R2) = similarity(a1, a2) * similarity(b1, b2).

Version
=======
0.2

License
=======

GNU GENERAL PUBLIC LICENSE Version 2.

Requirements
=============

* GNU Colecction Compiler (GCC) or Clang.
* GNU make (make).

Installation
=============

Clean and generate necessary files:

`$>make clean`

`$>make`

Usage
======

CMA command synopsis:

>./cma <cluster_directory> <graph_file> <matrix_file> [maximum_weight_M] [intra_weight]

where:

* <cluster_directory>: this directory contains the cluster files, corresponding to
a clustering in semEP format. 

* <graph_file>: file with the graph to study in semEP format.

* <matrix_file>: similarity matrix file with the similarities between all pairs relations. 

* [maximum_weight_M]: Optional argument, this is the value of the constant M for the edges in the
graph. See equation 8.26 in [2]. If no value is indicated, the default value is the average
weight of the edges of graph (without include bucles)

* [intra_weight]: Optional argument, this is the value of the theta with the importance
of the intra cluster edges in the graph. See equation 8.26 in [2]. If no value
is indicated, the default value is 0.9.

Running an example
===================

Example 1:

./cma example/nr_drug-target_graph-0.2553-0.1414-Clusters example/nr_drug-target_graph.txt example/nr_relation_sim_matrix.txt

Example 2:

./cma example/nr_drug-target_graph-0.2553-0.1414-Clusters example/nr_drug-target_graph.txt example/nr_relation_sim_matrix.txt 1 0.5

Contact
========

Maria-Esther Vidal (maria.vidal@tib.eu)

References
============

[1] Buluç, Aydın, et al. "Recent advances in graph partitioning."
Algorithm Engineering. Springer International Publishing, 2016. 117-158

[2] Gaertler, Marco. "Clustering." Network analysis (2005): 178-215.
web: https://i11www.iti.kit.edu/_media/projects/spp1126/files/g-c-05.pdf

[3] Newman, Mark EJ. "Modularity and community structure in networks."
Proceedings of the national academy of sciences 103.23 (2006): 8577-8582.

[4] https://en.wikipedia.org/wiki/Modularity_(networks)
