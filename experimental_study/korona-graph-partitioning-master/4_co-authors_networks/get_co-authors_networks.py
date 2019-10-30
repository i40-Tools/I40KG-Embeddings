#!/usr/bin/env python

import sys
import os
import openpyxl

START_ROW = 3

def load_edges(filename):
    edges = []
    with open(filename) as fd:
        for line in fd:
            tok = line.split("\t")
            t = tok[1]
            d = tok[0]
            edges.append((d, t))
    return edges

def load_graph(filename):
    edges = {}
    cont = 0
    with open(filename) as fd:
        n = int(fd.readline())
        for line in fd:
            tok = line.split("\t")
            assert(len(tok) == 4)
            t = tok[1]
            d = tok[0]
            edges[ (d, t)] = cont
            cont = cont + 1
        assert(len(edges) == n)
    return edges

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
        col = col + 1
    row = row + 1
    for line in fd:
        tok = line.split(",")
        assert(len(tok) == n)
        tok[n-1] = tok[n-1][:-1]
        col = 0
        for v in tok:
            M[row][col] = float(v)
            col = col + 1
        row = row + 1
    assert(col == n)
    fd.close()
    return M, n

def get_cluster_files(folder):
    files = []
    for r, d, f in os.walk(folder):
        for file in f:
            files.append(os.path.join(r, file))
    return files

def load_clusters(c_filenames):
    clusters = []
    for filename in c_filenames:
        edges = load_edges(filename)
        if len(edges) > 1:
            clusters.append(edges)
    return clusters


def compute_co_author_network(cluster, sim_matrix, graph_pos):
    co_author_network = {}
    n = len(cluster)
    for i in range(n-1):
        (author1, conf1) = cluster[i]
        pos1 = graph_pos[ (author1, conf1) ]
        for j in range(i+1, n):
            (author2, conf2) = cluster[j]
            pos2 = graph_pos[ (author2, conf2) ]
            sim = sim_matrix[pos1][pos2]
            if author1 != author2:
                if (author1, author2) in co_author_network:
                    if co_author_network[(author1, author2)] < sim:
                        co_author_network[(author1, author2)] = sim
                elif (author2, author1) in co_author_network:
                    if co_author_network[(author2, author1)] < sim:
                        co_author_network[(author2, author1)] = sim
                else:
                    if author1 < author2:
                        co_author_network[(author1, author2)] = sim
                    else:
                        co_author_network[(author2, author1)] = sim
                        
    return co_author_network

def get_all_co_author_network(clusters, sim_matrix, graph_pos):
    all_co_author_networks = []
    for cluster in clusters:
        co_author_network = compute_co_author_network(cluster, sim_matrix, graph_pos)
        all_co_author_networks.append(co_author_network)
    return all_co_author_networks

def get_co_authors_predictions(co_author_networks, co_author_graph):
    coauthorNetworks = []
    predictions = {}
    for key in co_author_networks:
        can = co_author_networks[key]
        for (author1, author2) in can:
            if author2 not in co_author_graph[author1]:
                value = can[(author1, author2)]
                if (author1, author2) in predictions:
                    if predictions[(author1, author2)] < value: 
                        predictions[(author1, author2)] = value 
                else:
                    predictions[(author1, author2)] = value 
        coauthorNetworks.append(can)
    pred = sorted(predictions.items(), key = lambda kv:(kv[1], kv[0]), reverse=True)
    return coauthorNetworks, pred
                        
def eliminate_duplicates(co_author_networks):
    can = {}
    n = len(co_author_networks)
    for i in range(n):
        current = co_author_networks[i]
        key = str(current.keys())
        if key not in can:
            can[key] =  current
        else:
            best = can[key]
            if cmp(best, current) == -1:
                best = current
                key = str(current.keys())
                can[key] = current
    return can

def get_author_name(author):
    tok = author.split("/")
    name = tok[-1].split(":")
    name.reverse()
    sname = ""
    for s in name[:-1]:
        sname += s + " "
    sname += name[-1]
    return sname

def load_co_authors_data(filename_xls):
    co_authors_graph = {}
    wb = openpyxl.load_workbook(filename_xls)
    sh = wb.get_active_sheet()
    cont = 0   
    author_count = 0 

    for r in sh.rows: 
        eachrow = []
        cont += 1     
        if (cont == 1):   
            continue
        num_author= int(r[2].value)   
        if (num_author > 0):
            for num_1 in range(1,num_author+1):  
                auth_1 = get_author_name( str(r[START_ROW+num_1].value) )
                for num_2 in range(1,num_author+1):  
                    auth_2 = get_author_name( str(r[START_ROW+num_2].value) )
                    if auth_1 not in co_authors_graph:
                        co_authors_graph[auth_1] = set()
                    co_authors_graph[auth_1].add(auth_2)
    return co_authors_graph

def write_all_co_author_networks(path_dir, coauthor_networks, pred):
    cont = 0
    if not os.path.exists(path_dir):
        os.makedirs(path_dir)
    for can in coauthor_networks:
        fnm = os.path.join(path_dir, "co-author_network_"+str(cont)+".tsv") 
        with open(fnm, "w") as fd:
            fd.write("Author_1\tAuthor_2\tWeight_of_connectivity\n")
            for (author1, author2) in can:
                fd.write(author1+"\t"+author2+"\t"+str(can[(author1, author2)])+"\n")
        cont += 1
    if len(pred) > 0:
        fnm = os.path.join(path_dir, "co-author_network_predictions.tsv") 
        with open(fnm, "w") as fd:
            fd.write("Author_1\tAuthor_2\tWeight_of_connectivity\n")
            for ((author1, author2), val) in pred:
                fd.write(author1+"\t"+author2+"\t"+str(val)+"\n")
        
        
def main(*args):
    graph_pos = load_graph(args[0])
    M, n = load_matrix_similarity(args[1])
    if len(graph_pos) != n:
        print("Error, in the number of edges of the graph")
        sys.exit(1)
    cluster_filenames = get_cluster_files(args[2])
    print("Number of filenames: "+str(len(cluster_filenames)))
    clusters = load_clusters(cluster_filenames)
    print("Number of clusters to consider: "+str(len(clusters)))
    co_author_graph = load_co_authors_data(args[3])
    print("Number of authors: "+str(len(co_author_graph))) 
    all_co_author_networks = get_all_co_author_network(clusters, M, graph_pos)
    filtered_co_author_networks = eliminate_duplicates(all_co_author_networks)
    print("Number of co-authors networks: "+str(len(filtered_co_author_networks)))
    coauthor_networks, pred = get_co_authors_predictions(filtered_co_author_networks, co_author_graph)
    assert(len(filtered_co_author_networks) == len(coauthor_networks))
    write_all_co_author_networks(args[4], coauthor_networks, pred)
    print("Done.")
    
if __name__ == '__main__':
    main(*sys.argv[1:])
