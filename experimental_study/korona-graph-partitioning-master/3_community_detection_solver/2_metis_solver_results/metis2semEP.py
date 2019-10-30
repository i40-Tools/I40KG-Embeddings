#!/usr/bin/env python

#
# Usage:
# ./metis2semEP.py <semEP input graph> <metis output> <output directory>
#
# Example: 
# ./metis2semEP.py ../semEP-with-constraints-master/datasets/author-conf/Auth-Conf_graph.txt results/metis_graph_p95.txt.part.3291 results_semEP_format/p95/
#

import sys, os, ntpath

def get_clusters(filename, lintera):
    dclus = {}
    fd = open(filename)
    elem = 0
    for line in fd:
        (d,t,cost,i) = lintera[elem]
        c = line[:-1]
        if c not in dclus:
            dclus[c] = []
            dclus[c].append( (d,t,i,cost) )
            #dclus[c].append( elem )
        else:
            dclus[c].append( (d,t,i,cost) )
            #dclus[c].append( elem )
        elem += 1 
    return dclus

def load_interactions(filename):
    interactions = []
    fd = open(filename)
    line = fd.readline()
    n = int(line[:-1])
    for line in fd:
        tok = line.split("\t")
        assert(len(tok) == 4)
        interactions.append((tok[0], tok[1], tok[3][:-1], tok[2]))
    assert(n == len(interactions))
    return interactions

def create_files(newdir, clusters, metis_file):
    if not os.path.exists(newdir):
        os.makedirs(newdir)
        
    for c in clusters:
        #cfile = "drugs-targets-"+c+"-t1-t2.txt"
        cfile = "metis-cluster-"+c+"-semEPformat.txt"
        newfile = os.path.join(newdir, cfile)
        #print("New file ",newfile)
        fd = open(newfile, "w")
        linterac  =  clusters[c]
        cnts_set = set()
        for (d,t,i,c) in linterac:
            fd.write(d+"\t"+t+"\t"+c+"\t"+i+"\n")
            cnts_set.add(i)
        if len(cnts_set) != 1:
            print("*Warning, cluster with mix of relations ******\n")
            print(cnts_set)
            #sys.exit(1)
        fd.close()
    return newdir

if __name__ == "__main__":
    iteracc = load_interactions(sys.argv[1])
    clusters = get_clusters(sys.argv[2], iteracc)
    #print(clusters)
    #print("Number of clusters ", len(clusters))
    new_dir = create_files(sys.argv[3], clusters, sys.argv[2])
    print("Done, results in the directory: "+new_dir+"\n")
    
