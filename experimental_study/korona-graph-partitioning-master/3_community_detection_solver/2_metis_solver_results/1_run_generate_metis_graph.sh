#!/bin/bash

percentiles=(80 85 90 95 98)
n=${#percentiles[@]}

for ((i=0;i<$n;i=i+1)); do
    echo "Running percentile :"${percentiles[$i]}
    python generate_metis_graph.py "../../2_relatedness_solver/output/matrix_p"${percentiles[$i]}".txt" "graphs/metis_graph_p"${percentiles[$i]}".txt" 
    echo "-----------------------------------"
done
echo "done"

