import os
import pandas as pd
import json
import matplotlib.pyplot as plt
from rdflib import Graph
import scipy
from sklearn.metrics.pairwise import cosine_similarity

# ==========analysis of similarity among standards belonging to the same framework=======
def get_standards(address_embedding, test_set):
    g = Graph()
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
    return array


def density_plot(array, config):
    similarity = []
    for key, value in array.items():
        for keyC, valueC in array.items():
            if key != keyC:
                sim = abs(1 - scipy.spatial.distance.cosine(value, valueC))
                similarity.append(sim)
    standard_similarity = pd.DataFrame()
    standard_similarity.insert(0, 'similarity', similarity)
    ax = standard_similarity["similarity"].plot.kde(bw_method=0.01)
    fig = ax.get_figure()
    fig.savefig('Density_plot/Density_plot_' + config + '.pdf', format='pdf', bbox_inches='tight')


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

for key, address_embedding in list_embedding.items():
    for fold in range(k):
        config = str(key) + str(fold)

        array = get_standards(address_embedding[fold], list_test_set[fold])
        density_plot(array, config)
