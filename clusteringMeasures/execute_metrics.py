#!/usr/bin/env python3

import os
from os.path import isfile, join
from os import listdir

list_cluster = [name for name in os.listdir(".") if os.path.isdir(name)]
print(list_cluster)

for name in list_cluster:
    commd = './cma ' +str(name)+'/clusters '+ str(name)+'/entities.txt '+str(name)+'/matrix_undirected_cosine.txt > '+str(name)+'/'+str(name)+'.txt'
    print(commd)
    os.system(commd)