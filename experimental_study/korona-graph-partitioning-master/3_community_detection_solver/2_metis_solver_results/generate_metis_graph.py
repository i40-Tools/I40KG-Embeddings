#!/usr/bin/env python

import sys, os

DECIMAL = 10000

def load_matrix_similarity(filename):
    matrix_created = False
    fd = open(filename)
    col = 0
    line = fd.readline()
    tok = line.split(",")
    n = len(tok)
    M = [[0.0 for x in range(n)] for x in range(n)]
    tok[n-1] = tok[n-1][:-1]
    row = 0
    for v in tok:
        M[row][col] = float(v)
        row = row + 1
    col = col + 1
    for line in fd:
        tok = line.split(",")
        assert(len(tok) == n)
        tok[n-1] = tok[n-1][:-1]
        row = 0
        for v in tok:
            M[row][col] = float(v)
            row = row + 1
        col = col + 1
    assert(col == n)
    fd.close()
    return M, n

def checking_symmetry(M, n):
    for i in range(n):
        for j in range(n):
            if M[i][j] != M[j][i]:
                print("Error the matrix is not symetric")
                sys.exit(1)
   
def generate_graph(M, n, filename):
    tmp_filename = filename+"_tmp.data"
    fd = open(tmp_filename, "w")
    nedges = 0
    cont = 0
    for i in range(n):
        added_first = False
        for j in range(n):
            cont += 1 
            k = M[i][j]*DECIMAL
            v = int(round(k))
            if (v > 0) and (i != j):
                if added_first:
                    fd.write(" "+str((j+1))+" "+str(v))
                else:
                    fd.write(str((j+1))+" "+str(v))
                    added_first = True
                nedges += 1
        fd.write("\n")
    assert(cont == n*n)
    assert(nedges % 2 == 0)
    fd.close()
    fdw = open(filename, "w")
    fdr = open(tmp_filename, "r")
    fdw.write(str(n)+" "+str(nedges//2)+" 001\n")
    for line in fdr:
        fdw.write(line)
    fdr.close()
    fdw.close()
    os.remove(tmp_filename)
    
if __name__ == "__main__":
    matrix, n = load_matrix_similarity(sys.argv[1])
    print("Input matrix "+sys.argv[1])
    checking_symmetry(matrix, n)
    generate_graph(matrix, n, sys.argv[2])
    print("Done, output graph: "+sys.argv[2]+" \n")
