#!/usr/bin/env python3

import os, sys, glob
from os import listdir
from os.path import isfile, join
import scipy
import numpy as np
import pandas as pd
import json
from sklearn.metrics.pairwise import cosine_similarity
from rdflib import Graph
from shutil import copyfile
from sklearn.cluster import KMeans


def call_semEP(threshold, cls_addres):
    DIR_SEM_EP = "semEP-node"
    current_path = os.path.dirname(os.path.realpath(__file__))
    th = "{:.4f}".format(float(threshold))

    commd = current_path + "/" + DIR_SEM_EP + " entities.txt matrix_undirected_cosine.tsv " + str(th)
    print('commd: '+commd)
    os.system(commd)
    pattern = 'entities'

    results_folder = glob.glob(current_path + "/" + pattern + "-*")
    #print(results_folder)
    onlyfiles = [os.path.join(results_folder[0], f) for f in listdir(results_folder[0]) if
                 isfile(join(results_folder[0], f))]
    count=0
    for filename in onlyfiles:
        #print(filename)
        key = "cluster-" + str(count)+ '.txt'
        copyfile(filename, cls_addres+'clusters/'+key)
        count+=1
    #dicc_clusters = get_dicc_clusters(onlyfiles)

    for r, d, f in os.walk(results_folder[0]):
        for files in f:
            os.remove(os.path.join(r, files))
        os.removedirs(r)
    return len(onlyfiles)

def get_dicc_clusters(onlyfiles):
    cont = 1
    dicc_clusters = {}
    for filename in onlyfiles:
        key = "Cluster-" + str(cont)
        dicc_clusters[key] = []
        with open(filename) as fd:
            for line in fd:
                e = line.rstrip()
                dicc_clusters[key].append(e)
        cont = cont + 1
    return dicc_clusters

# ==========analysis of similarity among standards belonging to the same framework=======
def get_standards(address_embedding, test_set):
    g = Graph()
    #g.parse("sto/relatedTo/training_set_relatedTo.nt", format="nt")
    g.parse(test_set, format="nt")
    len(g)
    qres = g.query(
        """PREFIX owl: <http://www.w3.org/2002/07/owl#>
           PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
           PREFIX sto: <https://w3id.org/i40/sto#>

        select distinct ?s where {
                ?s ?p ?o .
        }""")

    with open(address_embedding, 'rb') as f:
        # with open("sto/hasClassification/embeddings/TransR/entities_to_embeddings.json",'rb') as f:
        array = json.load(f)
    """
    new_dict = {}
    for row in qres:
        for key, value in array.items():
            if key == "%s" % row:
                tem = key
                # print(key)
                new_dict[tem] = array[key]

    with open('output_standard_same_framework.json', 'w') as f:
        # this would place the entire output on one line
        # use json.dump(lista_items, f, indent=4) to "pretty-print" with four spaces per indent
        json.dump(new_dict, f)

    # to read the file containing standards/frameworks along with their embeddings
    with open('output_standard_same_framework.json', 'r') as f:
        array = json.load(f)
    """
    return array

#============ Create standard/framework list===========
def create_entitie(array):
    list_n = [*array]
    entities = "\n".join(str(x) for x in list_n)
    n_ent = str(len(list_n))
    entity = open(ENTITIES_FILE, mode="w+")
    entity.write(n_ent + "\n" + entities)
    entity.close()
    return entities, n_ent, list_n

# === Compare each standard/framework with all the other standards/frameworks to build cosine similarity matrix==
def build_cosine_similarity(array, th):
    list_sim = []
    cosine_matrix = pd.DataFrame(index=list(array.keys()), columns=list(array.keys()))
    cosine_matrix = cosine_matrix.fillna(0.0)
    for key, value in array.items():
        for keyC, valueC in array.items():
            sim = abs(1 - scipy.spatial.distance.cosine(value, valueC))  # send the values of the standards/frameworks to cosine similarity function
            sim = round(sim, 5)
            list_sim.append(sim)
            cosine_matrix.at[key, keyC] = sim

    threshold = np.percentile(list_sim, th)
    print("percentil", threshold)
    for col in cosine_matrix.columns:
        cosine_matrix.loc[cosine_matrix[col] < threshold, [col]] = 0

    # === Save cosine similarity matrix with the structure SemEP need
    f = open('matrix_undirected_cosine.tsv', mode="w+")
    f.write(str(cosine_matrix.shape[0]) + "\n")
    f.close()
    cosine_matrix.to_csv('matrix_undirected_cosine.tsv', mode='a', sep=' ',
                         index=False, header=False, float_format='%.5f')
    return cosine_matrix, threshold


#Format required of the matrix to run clustering Measures
def matrix_to_run_cls_measures(cosine_matrix, entities, n_ent, cls_addres):
    cosine_matrix.to_csv(cls_addres + 'matrix_undirected_cosine.txt', mode='w+',
                         index=False, header=False, float_format='%.5f')
    cosine_matrix.to_csv(cls_addres + 'cosine_matrix.csv', index=False, header=False,  mode='w+', float_format='%.5f')
    with open(cls_addres + ENTITIES_FILE, "w+") as entity:
        entity.write(n_ent + "\n" + entities)

def call_KMeans(array, num_cls, list_n, cls_addres_km, cosine_matrix):
    vectors = []
    for key, value in array.items():
        vectors.append(value)
    X = np.array(vectors)
    kmeans = KMeans(n_clusters=num_cls, random_state=0).fit(cosine_matrix)
    # ==Save each partition standads into a file==
    list_cluster = pd.DataFrame()
    list_cluster.insert(0, 'standards', list_n)
    list_cluster.insert(1, 'clusters', kmeans.labels_)
    for i in range(num_cls):
        cls_i = list_cluster.loc[list_cluster['clusters'] == i]
        cls_i['standards'].to_csv(cls_addres_km + 'clusters/cluster-' + str(i) + '.txt', index=False)

def METIS_Undirected_MAX_based_similarity_graph(cosine_matrix, cls_addres_metis):
    metislines = []
    nodes = {"name": [], "id": []}
    kv = 1
    edges = 0
    for i, row in cosine_matrix.iterrows():
        val = ""
        ix = 1
        ledges = 0
        found = False
        for k in row.keys():
            if i != k and row[k] > 0:
                val += str(ix) + " " + str(int(row[k] * 100000)) + " "
                # Only one edge is counted between two nodes, i.e., (u,v) and (v, u) edges are counted as one
                # Self links are also ignored, Notive ix>kv
                # if ix > kv:
                ledges += 1
                found = True
            ix += 1
        if found:
            # This node is connected
            metislines.append(val.strip())
            edges += ledges
            nodes["name"].append(i)
            nodes['id'].append(str(kv))
        else:
            # disconnected RDF-MTs are given 10^6 value as similarity value
            metislines.append(str(kv) + " 100000")
            edges += 1
            # ---------
            nodes["name"].append(i)
            nodes['id'].append(str(kv))
            print(i)
            print(str(kv))

        kv += 1
    nodes = pd.DataFrame(nodes)
    #print(edges)
    numedges = edges // 2
    # == Save filemetis.graph to execute METIS algorithm ==
    ff = open(cls_addres_metis + 'metis.graph', 'w+')
    ff.write(str(cosine_matrix.shape[0]) + " " + str(numedges) + " 001\n")
    met = [m.strip() + "\n" for m in metislines]
    ff.writelines(met)
    ff.close()
    return nodes


def call_metis(num_cls, nodes, cls_addres_metis):
    # !sudo docker run -it --rm -v /media/rivas/Data1/Data-mining/KCAP-I40KG-Embeddings/I40KG-Embeddings/result/TransD/metis:/data kemele/metis:5.1.0 gpmetis metis.graph 2
    EXE_METIS = "sudo docker run -it --rm -v "
    DIR_METIS = ":/data kemele/metis:5.1.0 gpmetis"
    cls_addres = cls_addres_metis[:-1]
    commd = EXE_METIS + cls_addres + DIR_METIS + " metis.graph " + str(num_cls)
    print(commd)
    os.system(commd)
    parts = open(cls_addres_metis + 'metis.graph.part.' + str(num_cls)).readlines()
    parts = [p.strip() for p in parts]
    # == Save each partition standads into a file ==
    i = 0
    partitions = dict((str(k), []) for k in range(num_cls))
    for p in parts:
        name = nodes.iat[i, 0]
        i += 1
        partitions[str(p)].append(name)

    i = 0
    count = 0
    for p in partitions:
        if len(partitions[p]) == 0:
            continue
        count += len(partitions[p])
        f = open(cls_addres_metis + 'clusters/cluster-' + str(i) + '.txt', 'w+')
        [f.write(l + '\n') for l in partitions[p]]
        f.close()
        i += 1


def get_measure(cls_measure, folder):
    measure = ' ./cma'+folder + '/clusters/ ' + folder+'/entities.txt ' + folder +'/matrix_undirected_cosine.txt >' + folder+'.txt'
    c_measure = cls_measure[:-1]
    print(c_measure)
    print(measure)
    os.system(c_measure + measure)


ENTITIES_FILE = "entities.txt"
cls_measure = 'clusteringMeasures/'
list_embedding = {}
list_embedding['TransD'] = ['embeddings/training_set_relatedTo/TransD/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo1/TransD/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo2/TransD/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo3/TransD/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo4/TransD/entities_to_embeddings.json']
list_embedding['TransE'] = ['embeddings/training_set_relatedTo/TransE/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo1/TransE/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo2/TransE/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo3/TransE/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo4/TransE/entities_to_embeddings.json']
list_embedding['TransH'] = ['embeddings/training_set_relatedTo/TransH/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo1/TransH/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo2/TransH/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo3/TransH/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo4/TransH/entities_to_embeddings.json']
list_embedding['TransR'] = ['embeddings/training_set_relatedTo/TransR/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo1/TransR/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo2/TransR/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo3/TransR/entities_to_embeddings.json',
                            'embeddings/training_set_relatedTo4/TransR/entities_to_embeddings.json']
list_test_set = ['test_set/test_set_relatedTo0.nt', 'test_set/test_set_relatedTo1.nt',
                 'test_set/test_set_relatedTo2.nt', 'test_set/test_set_relatedTo3.nt',
                 'test_set/test_set_relatedTo4.nt']
k = 5
current_path = os.path.dirname(os.path.realpath(__file__))


for key, address_embedding in list_embedding.items():
    th = 85
    for fold in range(k):
        cls_addres = cls_measure + 'SemEP' + str(key) + str(fold) + '/'
        cls_addres_km = cls_measure + 'KMeans' + str(key) + str(fold) + '/'
        cls_addres_metis = current_path+'/'+cls_measure + 'METIS' + str(key) + str(fold) + '/'
        if not os.path.exists(cls_addres):
            os.makedirs(cls_addres)
            os.makedirs(cls_addres + 'clusters/')
        if not os.path.exists(cls_addres_km):
            os.makedirs(cls_addres_km)
            os.makedirs(cls_addres_km + 'clusters/')
        if not os.path.exists(cls_addres_metis):
            os.makedirs(cls_addres_metis)
            os.makedirs(cls_addres_metis + 'clusters/')
        if key == 'TransH':
            th = 50
        if key == 'TransR':
            th = 75
        array = get_standards(address_embedding[fold], list_test_set[fold])
        entities, n_ent, list_n = create_entitie(array)
        cosine_matrix, threshold = build_cosine_similarity(array, th)
        matrix_to_run_cls_measures(cosine_matrix, entities, n_ent, cls_addres)
        matrix_to_run_cls_measures(cosine_matrix, entities, n_ent, cls_addres_km)
        matrix_to_run_cls_measures(cosine_matrix, entities, n_ent, cls_addres_metis)

        num_cls = call_semEP(threshold, cls_addres)
        call_KMeans(array, num_cls, list_n, cls_addres_km, cosine_matrix)
        nodes = METIS_Undirected_MAX_based_similarity_graph(cosine_matrix, cls_addres_metis)
        if key == 'TransH' and fold == 4:
            continue
        try:
            call_metis(num_cls, nodes, cls_addres_metis)
        except FileNotFoundError:
            print(fold)