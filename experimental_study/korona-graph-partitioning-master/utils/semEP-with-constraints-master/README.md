# semEP with constraints

## 1.  About

semEP is an edge partitioning approach that combines
a data mining framework for link prediction, semantic knowledge
(similarities) from ontologies, and an algorithmic approach
to partition the edges of a heterogeneous graph.

For more information about semEP see:

Guillermo Palma, Maria-Esther Vidal and Louiqa Raschid.
*Drug-Target Interaction Prediction Using Semantic Similarity and Edge
Partitioning*. Proceedings of the 12th International Semantic Web
Conference (ISWC 2013). Italy. 2014. [(PDF)](http://ldc.usb.ve/~gpalma/papers/semEP-ISWC14.pdf)

## 2. Content

* AUTHORS: list of contributors of the semEP project.
* doc: Documentation about semEP. 
* LICENSE: GPL version 2.
* Makefile: builds semEP.
* README.md: this file.
* src: source code.
* datasets: datasets to test semEP
** datasets/yamanishi: Drug-Target dataset proposed by Yamanishi [1].
** datasets/iDrug-Target: Drug-Target dataset proposed by Xuan Xiao et al. [2].
* VERSION: semEP version.
  
[1] K. Bleakley and Y. Yamanishi.
*Supervised prediction of drug-target interactions
using bipartite local models*.
Bioinformatics, 25(18):2397-2403, 2009.

[2] Xiao, X., Min, J.-L., Lin, W.-Z., Liu, Z., Cheng, X., and Chou, K.-C
"iDrug-Target: predicting the interactions between drug compounds 
and target proteins in cellular networking via benchmark dataset 
optimization approach"
Journal of Biomolecular Structure and Dynamics 33 (2015), 1–13.

## 3. License

GNU GENERAL PUBLIC LICENSE Version 2.

## 4. Requirements

* GNU Colecction Compiler (GCC) or Clang.
* GNU make (make).

semEP has been tested on FreeBSD, GNU/Linux, and OS X.

## 5. Installation

Clean and generate necessary files:

`$>make clean`

`$>make`

The result is that executable file 'semEP' will be created.

## 6. Usage

semEP has 7 mandatory command line arguments and it has
2 optional command line arguments. Mandatory means that without specifying this
argument, the program won't work.

semEP command synopsis:

`semEP [-c] [-p] <-l left threshold> <-r right threshold> <left matrix>  <left vertices> <right matrix> <right vertices> <bipartite graph>`

where mandatory arguments are:

* <bipartite grap>: file with the bipartite graph (BPG) to study.
* <left threshold>: threshold of similarity between the left side vertices of the BPG.
* <right threshold>: threshold of similarity between the right side vertices of the BPG.
* <left matrix>: similarity matrix file with the similarities between the left side vertices of the BPG.
* <-l left vertices>: file with the list of vertices of the left side of the BPG. The order of the vertices corresponds to the <left matrix>. 
* <-r right matrix>: similarity matrix file with the similarities between the right side vertices of the BPG.
* <right vertices>: file with the list of vertices of the right side of the BPG. The order of the vertices corresponds to the <right matrix>. 

where optional arguments are:

* [-c]: apply edge restriction. When you uses this option, in the output clustering,
  two edges never are in the same cluster if they have different relationships.   
* [-p]: get the predicted links from the clusters. 

## 7. Running some samples

### 7.1 Computing the partitions of a drug-target bipartite graph on Yamanishi's enzyme dataset:

>./semEP -l 0.2174 -r 0.0198 datasets/yamanishi/e/e_matrix_drugs.txt datasets/yamanishi/e/e_drugs.txt datasets/yamanishi/e/e_matrix_targets.txt datasets/yamanishi/e/e_targets.txt datasets/yamanishi/e/e_drug-target_graph.txt

### 7.2 Computing the partitions of a drug-target bipartite graph and get the predicted links on Yamanishi's nuclear receptor dataset:

>./semEP -p -l 0.3061 -r 0.1614 datasets/yamanishi/nr/nr_matrix_drugs.txt datasets/yamanishi/nr/nr_drugs.txt datasets/yamanishi/nr/nr_matrix_targets.txt datasets/yamanishi/nr/nr_targets.txt datasets/yamanishi/nr/nr_drug-target_graph.txt

### 7.3 Computing the partitions using relationship constraints, of a drug-target bipartite graph on Xiao's enzyme dataset:

>./semEP -c -l 0.25 -r 0.03 datasets/iDrug-Target/e/e_matrix_drugs.txt datasets/iDrug-Target/e/e_drugs.txt datasets/iDrug-Target/e/e_matrix_targets.txt datasets/iDrug-Target/e/e_targets.txt datasets/iDrug-Target/e/e_drug-target_graph.txt

### 7.4 Computing the partitions using relationship constraints, and get the predicted links of a drug-target bipartite graph on Xiao's ion channel dataset:

>./semEP -l 0.4 -r 0.01 -p -c datasets/iDrug-Target/ic/ic_matrix_drugs.txt datasets/iDrug-Target/ic/ic_drugs.txt datasets/iDrug-Target/ic/ic_matrix_targets.txt datasets/iDrug-Target/ic/ic_targets.txt datasets/iDrug-Target/ic/ic_drug-target_graph.txt

## 8 semEP input

### 8.1. File format of the bipartite graph

A bipartite graph is a graph whose vertices can be divided into
two disjoint sets *L* and *R* such that every edge connects a
vertex in *L* to one in *R*. We can denote a bipartite graph
as *G=(L,R,E)*, with *E* denoting the edges of the graph. We call
the set *L* as *the left side* of the bipartite graph, and the set *R*
as *the right side* of the bipartite graph. 

The first line of the bipartite graph file, contains the number of edges.
The following lines in the file shows the edges data, where
each line corresponds to a edge and contains the node of the left
side, the node of the right side, the relationship type, and the cost of the edge.

	[number of edges n]
	[right-node edge-1][TAB][lef-tnode edge-1][TAB][relationship-1][TAB][edge-cost-1]
	..
	..
	..
	[right-node edge-n][TAB][left-node edge-n][TAB][relationship-n][TAB][edge-cost-n]

### 8.2. File format of the similarity matrix

Files with the matrices must contain the similarities between the vertices of the left side,
or the right side. The file format is as follows:

	[number of rows and columns]
	[sim vertex-1 vertex-1][SPC]...[SPC][sim vertex-1 vertex-n]
	..
	..
	..
	[sim vertex-n vertex-1][SPC]...[SPC][sim vertex-n vertex-n]

### 8.3. File format of the list vertices

The file format is as follows:

	[number of vertices n]
	[vertex 1]
	..
	..
	..
	[vertex n]

Where *[vertex x]* is the identifier of vertex in the left side or right 
side of the bipartite graph. The order of each vertex correspond to the
position of the vertex in the similarity matrix.

## 9. semEP output

When semEP is executed without the option [-p], then semEP produces as output
a directory with suffix *-Clusters* that contains the clusters with the
edge partitioning of the bipartite graph. Each cluster correspond to a file on the directory.

Additionally, when is used the option [-p], semEP creates a file with
suffix *-Predictions.txt* that contains the semEP predictions for each cluster
and the probability of each prediction. 

## 10. Contact

I hope you find semEP an useful tool. Please, let me know
any comment, problem, bug, or suggestion.

Guillermo Palma
[palma at l3s dot de](mailto:palma@l3s.de)
