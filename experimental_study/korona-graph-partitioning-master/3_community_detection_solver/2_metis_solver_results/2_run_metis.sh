#!/bin/bash

percentiles=(80 85 90 95 98)
n=${#percentiles[@]}
clusters=(890 1269 1933 3291 5414)

rm results/*
for ((i=0;i<$n;i++)); do
    echo "Running : gpmetis graphs/metis_graph_p"${percentiles[$i]}".txt"  ${clusters[$i]} 
    time gpmetis "graphs/metis_graph_p"${percentiles[$i]}".txt"  ${clusters[$i]} 
done
mv graphs/*part* results/
echo "done metis results"
echo ""
echo "----------------------------------------"
echo "Getting the results in semEP format"
echo "----------------------------------------"
echo ""
for ((i=0;i<$n;i++)); do
    echo " Running metis2semEP.py in percentile"${percentiles[$i]}
    ./metis2semEP.py ../../1_data_processing/output/Auth-Conf_graph-descrp.txt "results/metis_graph_p"${percentiles[$i]}".txt.part."${clusters[$i]} "results_semEP_format/p"${percentiles[$i]}
done
echo "done metis results in semEP format in the folder results_semEP_format"
