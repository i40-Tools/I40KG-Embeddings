#!/bin/bash

echo ""
echo "Step 1 Getting ISWC data"
time python 1.\ filter-nt\ .py 
echo "--------------------------------------"

echo "Step 2 Filtering the data"
time python 2.\ rdflib2excel.py
echo "--------------------------------------"

echo "Step 3 Computing the similarities between conferences"
time python 3.\ similarities.py
echo "--------------------------------------"

echo "Step 4 Computing the similarities between authors"
time python 4.\ author\ similarity.py 
echo "--------------------------------------"

echo "Step 5 Getting the semEP input data"
time python 5.\ get_semEP_data.py 
echo "--------------------------------------"
