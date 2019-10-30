#!/usr/bin/env python

import sys, os

def get_elems(filename):
    fdd = open(filename, "r")
    fdd.readline()
    elems = []
    for line in fdd:
        l = line[:-1]
        elems.append(l)
    return elems

def load_similarity(filename, lelems):
    fd = open(filename, "r")
    line = fd.readline()
    n = int(line[:-1])
    
    assert(len(lelems) == n)
    row = 0
    col = 0
    simDic = {}
    for line in fd:
        tok = line.split(" ")
        last = tok[-1][:-1]
        tok[-1] = last
        assert(len(tok) == n)
        col = 0
        for x in tok:
            a = lelems[row]
            b = lelems[col]
            key = (a, b)
            assert(key not in simDic)
            simDic[key] =  float(x)
            col = col + 1
        row = row + 1
        
    return simDic

def load_interactions(filename):
    interactions = []
    fd = open(filename)
    n = int(fd.readline())
    cont = 0
    for line in fd:
        tok = line.split("\t")
        assert(len(tok) == 4)
        t = tok[1]
        d = tok[0]
        cnst = tok[2] # Constraint
        interactions.append((d, t, cnst))
        cont += 1
    assert(cont == n)
    return interactions

def generate_matrix(simD, simT, inter, td, tt):
    n = len(inter)
    simInter = [[0.0 for x in range(n)] for x in range(n)]
    for i in range(n):
        for j in range(n):
            (d1, t1, c1) = inter[i]
            (d2, t2, c2) = inter[j]
            sd = simD[(d1, d2)]
            st = simT[(t1, t2)]
            if ((sd < td) or (st < tt)) or (c1 != c2):
                simInter[i][j] = 0.0
            else:
                simInter[i][j] = sd*st
    return (simInter, n)

def print_matrix(filename, simM, nele):
    fd = open(filename, "w")
    for i in range(nele):
        for j in range(nele-1):
            fd.write(str(simM[i][j])+",")
            
        fd.write(str(simM[i][nele-1])+"\n")
                
def get_simetric_matrix(M, n):
    S = [[0.0 for x in range(n)] for x in range(n)]
    for i in range(n):
        for j in range(n):
            a  = M[i][j]
            b  = M[j][i]
            if (a != b):
                r = (a+b)/2.0
            else:
                r = a
            S[i][j] = r
            S[j][i] = r
    return S

def check_is_symetric(A, n):
    for i in range(n):
        for j in range(n):
            if A[i][j] != A[j][i] :
                #print("Error, matrix not is symetric")
                #print(i, j, A[i][j], A[j][i])
                return False
    #print("Yes, matrix is symetric")
    return True

if __name__ == "__main__":
    drugs = get_elems(sys.argv[4])
    simDrugs = load_similarity(sys.argv[3], drugs)
    #print(drugs)
    #print("*********")
    #print(simDrugs)
    targets = get_elems(sys.argv[6])
    simTargets = load_similarity(sys.argv[5], targets)
    #print(targets)
    #print("*********")
    #print(simTargets)
    interacc = load_interactions(sys.argv[7])
    #print(interacc)
    td = float(sys.argv[1])
    tt = float(sys.argv[2])
    (simInterMatrix, n) = generate_matrix(simDrugs, simTargets, interacc, td, tt)
    #print(simInterMatrix)
    if not check_is_symetric(simInterMatrix, n) :
        simInterMatrix = get_simetric_matrix(simInterMatrix, n)
        if not check_is_symetric(simInterMatrix, n) :
            print("Error in get a symetric matrix")
            sys.exit(1)
    print_matrix(sys.argv[8], simInterMatrix, n)
    print("Done! in "+sys.argv[8])
