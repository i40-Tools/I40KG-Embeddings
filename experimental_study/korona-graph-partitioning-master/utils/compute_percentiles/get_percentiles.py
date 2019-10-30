#!/usr/bin/env python3

import sys, os, traceback, numpy

def load_matrix(filenamepath):
    cl = 0
    filename = open(filenamepath, "r")
    line = filename.readline()
    n = int(line[:-1])
    M = [[0.0 for x in range(n)] for x in range(n)]
    col = 0
    for line in filename:
        cl += 1
        tok = line.split(" ")
        assert (len(tok) == n)
        tok[n-1]  = tok[n-1][:-1]
        row = 0
        for v in tok:
            M[row][col] = float(v)
            row = row + 1
        col = col + 1
    assert(cl == n)
    return M, n

def get_sim_values(M, n):
    cont = 0
    sim = []
    for i in range(n-1):
        j = i+1
        while(j < n):
            value = (M[i][j]+M[j][i])/2.0
            sim.append(value)
            cont +=1
            j += 1
    assert(cont == (n*n-n)/2)
    return sim

def get_file_name(dirFile):
    base = os.path.basename(dirFile)
    return os.path.splitext(base)[0]

def get_threshold(a):
    print("Min: {0:.4f}".format(numpy.amin(a)))
    print("Max: {0:.4f}".format(numpy.amax(a)))
    print("Average: {0:.4f}".format(numpy.average(a)))
    print("Median: {0:.4f}\n".format(numpy.median(a)))
    assert( numpy.percentile(a, 50) ==  numpy.median(a) )
    values = [10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 98]
    print("Percentile\tSimilarity")
    for v in values:
        r = numpy.percentile(a, v)
        print("{0}\t{1:.4f}".format(v, r))
    
if __name__ == "__main__":
    M,n = load_matrix(sys.argv[1])
    sim = get_sim_values(M, n)
    get_threshold(sim)
