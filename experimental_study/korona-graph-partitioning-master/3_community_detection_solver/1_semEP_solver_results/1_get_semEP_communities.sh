#!/bin/bash

authors=(0.0882 0.1061 0.1379 0.2000 0.3333)
n=${#authors[@]}

conferences=(0.1057 0.1211 0.1290 0.1479 0.1586)

percentiles=(80 85 90 95 98)

rm -rf "semEP_output/"*
for ((i=0;i<$n;i++)); do
    echo "Running semEP percentil "${percentiles[i]}
    ../../utils/semEP-with-constraints-master/semEP -p -l ${authors[$i]} -r ${conferences[$i]} ../../1_data_processing/output/Auth_matrix.txt ../../1_data_processing/output/author-descrp.txt ../../1_data_processing/output/Conf_matrix.txt ../../1_data_processing/output/conf-descrp.txt ../../1_data_processing/output/Auth-Conf_graph-descrp.txt > "Auth-Conf_graph-descrp-"${authors[$i]}"-"${conferences[$i]}"-semEP_output.txt"
    mkdir "semEP_output/p"${percentiles[$i]}"/"
    mv "Auth-Conf_graph-descrp-"${authors[$i]}"-"${conferences[$i]}"-"* "semEP_output/p"${percentiles[$i]}"/"
done

echo "semEP is done"
