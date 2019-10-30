#!/bin/bash

authors=(0.0882 0.1061 0.1379 0.2000 0.3333)
n=${#authors[@]}
conferences=(0.1057 0.1211 0.1290 0.1479 0.1586)
percentiles=(80 85 90 95 98)

for ((i=0;i<$n;i=(i+1))); do
    echo "Computing relation matrix with percentil "${percentiles[i]}
    time python sim_matrix_with_rel_constraints.py ${authors[$i]} ${conferences[$i]} ../1_data_processing/output/Auth_matrix.txt ../1_data_processing/output/author-descrp.txt  ../1_data_processing/output/Conf_matrix.txt ../1_data_processing/output/conf-descrp.txt ../1_data_processing/output/Auth-Conf_graph-descrp.txt output/matrix_"p"${percentiles[$i]}".txt"
done
echo "Done"
