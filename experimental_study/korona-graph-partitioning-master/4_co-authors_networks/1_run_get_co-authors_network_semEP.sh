#!/bin/bash

rm -rf semEP_co-author-networks/p80
echo "Running percentile 80"
python get_co-authors_networks.py  ../1_data_processing/output/Auth-Conf_graph-descrp.txt ../2_relatedness_solver/output/matrix_p80.txt  ../3_community_detection_solver/1_semEP_solver_results/semEP_output/p80/Auth-Conf_graph-descrp-0.0882-0.1057-Clusters/  ../1_data_processing/output/papers_authors.xlsx semEP_co-author-networks/p80

rm -rf semEP_co-author-networks/p85
echo "Running percentile 85"
python get_co-authors_networks.py  ../1_data_processing/output/Auth-Conf_graph-descrp.txt ../2_relatedness_solver/output/matrix_p85.txt ../3_community_detection_solver/1_semEP_solver_results/semEP_output/p85/Auth-Conf_graph-descrp-0.1061-0.1211-Clusters/  ../1_data_processing/output/papers_authors.xlsx semEP_co-author-networks/p85

rm -rf semEP_co-author-networks/p90
echo "Running percentile 90"
python get_co-authors_networks.py  ../1_data_processing/output/Auth-Conf_graph-descrp.txt ../2_relatedness_solver/output/matrix_p90.txt ../3_community_detection_solver/1_semEP_solver_results/semEP_output/p90/Auth-Conf_graph-descrp-0.1379-0.1290-Clusters/  ../1_data_processing/output/papers_authors.xlsx semEP_co-author-networks/p90

rm -rf semEP_co-author-networks/p95
echo "Running percentile 95"
python get_co-authors_networks.py  ../1_data_processing/output/Auth-Conf_graph-descrp.txt ../2_relatedness_solver/output/matrix_p95.txt ../3_community_detection_solver/1_semEP_solver_results/semEP_output/p95/Auth-Conf_graph-descrp-0.2000-0.1479-Clusters/  ../1_data_processing/output/papers_authors.xlsx semEP_co-author-networks/p95

rm -rf semEP_co-author-networks/p98
echo "Running percentile 98"
python get_co-authors_networks.py  ../1_data_processing/output/Auth-Conf_graph-descrp.txt ../2_relatedness_solver/output/matrix_p98.txt ../3_community_detection_solver/1_semEP_solver_results/semEP_output/p98/Auth-Conf_graph-descrp-0.3333-0.1586-Clusters/  ../1_data_processing/output/papers_authors.xlsx semEP_co-author-networks/p98
