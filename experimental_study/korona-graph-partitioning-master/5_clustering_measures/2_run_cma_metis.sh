#!/bin/bash

percentiles=(80 85 90 95 98)
n=${#percentiles[@]}

for ((i=0;i<$n;i++)); do
    echo "Running percentil "${percentiles[i]}
    ../utils/clusteringMeasures/cma  "../3_community_detection_solver/2_metis_solver_results/results_semEP_format/p"${percentiles[$i]} ../1_data_processing/output/Auth-Conf_graph-descrp.txt "../2_relatedness_solver/output/matrix_p"${percentiles[($i)]}".txt" > "metis_cm_output/metis_cma_"${percentiles[($i)]}".txt" 
done
echo  "Done"
