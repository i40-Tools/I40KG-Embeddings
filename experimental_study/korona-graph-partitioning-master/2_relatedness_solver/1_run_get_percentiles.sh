#!/bin/bash

echo "Computing author percentiles"
python ../utils/compute_percentiles/get_percentiles.py  ../1_data_processing/output/Auth_matrix.txt  > output/Author_percentiles.txt
echo "--------------------"
echo "Computing conference percentiles"
python ../utils/compute_percentiles/get_percentiles.py  ../1_data_processing/output/Conf_matrix.txt  > output/Conference_percentiles.txt
