# Korona

----
## 1.  About

Korona is a framework to discover unknown co-authorship relations in a knowledge graph. Korona relies on the relatedness between scholarly entities and graph partitioning methods to identify communities composed
of researchers that work on similar topics.  The Korona implementation of the Korona architecture corresponds to the described in the paper [1].

[1] Vahdati, S., Palma, G., Nath, R. J., Lange, C., Auer, S., & Vidal, M. E.
"Unveiling scholarly communities over knowledge graphs."
International Conference on Theory and Practice of Digital Libraries.
Springer, Cham, 2018

----
## 2. Content

* 1\_data\_processing:  This directory contains the programs to load the input data and create a scholarly KG. Furthermore, the similarities between the scholarly entities are computed.

* 2\_relatedness\_solver: The software computes the relatedness between the scholarly entities in a scholarly knowledge graph.

* 3\_community\_detection\_solver: The software in this folder uses community detection solvers to obtain communities of related co-authors.

* 4\_co-authors\_networks: This folder contains the software to compute the co-author networks.

* 5\_clustering\_measures: This folder contains the programs to compute the measures of quality of the clustering obtained by Korona using the community detection methods

* 6\_draw\_clustering\_measures: This folder contains the scripts for the generation of radar charts. The radar charts show the comparison of the Korona results using the solvers Metis and semEP.

* doc: Documentation about Korona. 

* utils: Tools for the Korona execution. 

* utils/clusteringMeasures: This program computes the clusterings measures for the communities obtained by Korona. 

* utils/compute\_percentiles: This program computes the similarity percentiles in a set of similarity values. 

* utils/semEP-with-constraints-master: Community detection solver.

____
## 3. License

MIT License

____
## 4. Requirements

* Python 2

The following Python 2 libraries are required:

* rdflib
* openpyxl
* bs4
* pygal

----
## 5. Installation

The libraries indicated in the above section must be
installed using the pip or the package manager of the operating system.

The instructions for the installation of the tools in the folder
"utils" are in the readme file of each tool.

-----
## 6. Usage

 The execution order corresponds to the name of each directory.
 Each directory contains one or more bash scripts.  The bash scripts
 must be executed in the order indicated in the name of the bash scripts.

----
## 7. Contact

Please, let me know any comment, problem, bug, or suggestion.

Maria-Esther Vidal
[maria dot vidal at tib dot eu](mailto:maria.vidal@tib.eu)

