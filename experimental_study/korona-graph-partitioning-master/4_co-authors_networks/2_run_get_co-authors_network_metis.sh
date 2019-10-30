#!/bin/bash

rm -rf metis_co-author-networks/p80
echo "Running percentile 80"
python get_co-authors_networks.py  ../1_data_processing/output/Auth-Conf_graph-descrp.txt ../2_relatedness_solver/output/matrix_p80.txt  ../3_community_detection_solver/2_metis_solver_results/results_semEP_format/p80/  ../1_data_processing/output/papers_authors.xlsx metis_co-author-networks/p80

rm -rf metis_co-author-networks/p85
echo "Running percentile 85"
python get_co-authors_networks.py  ../1_data_processing/output/Auth-Conf_graph-descrp.txt ../2_relatedness_solver/output/matrix_p85.txt ../3_community_detection_solver/2_metis_solver_results/results_semEP_format/p85/  ../1_data_processing/output/papers_authors.xlsx metis_co-author-networks/p85

rm -rf metis_co-author-networks/p90
echo "Running percentile 90"
python get_co-authors_networks.py  ../1_data_processing/output/Auth-Conf_graph-descrp.txt ../2_relatedness_solver/output/matrix_p90.txt ../3_community_detection_solver/2_metis_solver_results/results_semEP_format/p90/  ../1_data_processing/output/papers_authors.xlsx metis_co-author-networks/p90

rm -rf metis_co-author-networks/p95
echo "Running percentile 90"
python get_co-authors_networks.py  ../1_data_processing/output/Auth-Conf_graph-descrp.txt ../2_relatedness_solver/output/matrix_p95.txt ../3_community_detection_solver/2_metis_solver_results/results_semEP_format/p95/  ../1_data_processing/output/papers_authors.xlsx metis_co-author-networks/p95

rm -rf metis_co-author-networks/p98
echo "Running percentile 98"
python get_co-authors_networks.py  ../1_data_processing/output/Auth-Conf_graph-descrp.txt ../2_relatedness_solver/output/matrix_p98.txt ../3_community_detection_solver/2_metis_solver_results/results_semEP_format/p98/  ../1_data_processing/output/papers_authors.xlsx metis_co-author-networks/p98

