#!/bin/bash

authors=(0.0882 0.1061 0.1379 0.2000 0.3333)
n=${#authors[@]}
conferences=(0.1057 0.1211 0.1290 0.1479 0.1586)
percentiles=(80 85 90 95 98)

for ((i=0;i<$n;i++)); do
    echo "Running percentil "${percentiles[i]}
    ../utils/clusteringMeasures/cma "../3_community_detection_solver/1_semEP_solver_results/semEP_output/p"${percentiles[$i]}"/Auth-Conf_graph-descrp-"${authors[$i]}"-"${conferences[$i]}"-Clusters" ../1_data_processing/output/Auth-Conf_graph-descrp.txt "../2_relatedness_solver/output/matrix_p"${percentiles[($i)]}".txt" > "semEP_cm_output/semEP_cma_"${percentiles[($i)]}".txt" 
done
echo  "Done"
