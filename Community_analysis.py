import pandas as pd
import os
from rdflib import Graph
from os.path import isfile, join
from os import listdir
import matplotlib.pyplot as plt

def load_cluster(cluster_addres, n_cls):
    cluster_list = []
    input_path = cluster_addres + 'clusters/'
    for i in range(n_cls):
        cls = input_path + 'cluster-' + str(i) + '.txt'
        c_i = pd.read_csv(cls, delimiter=",", header=None)
        c_i.columns = ['o']
        cluster_list.append(c_i)
    return cluster_list


def load_test_set(test_set_i):
    g = Graph()
    g.parse(test_set_i, format="nt")

    test_set = pd.DataFrame(columns=['s', 'p', 'o'])
    qres = g.query(
        """PREFIX owl: <http://www.w3.org/2002/07/owl#>
            PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
            PREFIX sto: <https://w3id.org/i40/sto#>
            select distinct ?s ?p ?o where {
                ?s ?p ?o .
            }""")
    sub = []
    pre = []
    obj = []
    for row in qres:
        sub.append(str("%s" % row['s']))
        pre.append(str("%s" % row['p']))
        obj.append(str("%s" % row['o']))
    test_set.s = sub
    test_set.p = pre
    test_set.o = obj
    #print(test_set.shape)
    subject = pd.DataFrame(test_set['s'])
    subject.columns = ['o']
    return test_set, subject


def compute_accuracy(cluster_list, test_set, subject):
    accuracy = 0
    count = 0
    for cls in cluster_list:
        intersected_df = pd.merge(cls, subject, how='inner', on='o')
        intersected_df = intersected_df.drop_duplicates().reset_index(drop=True)
        if intersected_df.shape[0] == 0:
            #print('zero-division')
            continue
        relation = pd.DataFrame(columns=['s', 'p', 'o'])
        for i in range(intersected_df.shape[0]):
            if intersected_df.o.iloc[i] in list(test_set.s):
                index_0 = test_set[test_set.s == intersected_df.o.iloc[i]].index
                lst = test_set.loc[index_0]
                relation = pd.concat([relation, lst], ignore_index=True)
        relation = pd.DataFrame(relation['o'])
        relation = relation.drop_duplicates().reset_index(drop=True)

        intersected_relatedTo = pd.merge(cls, relation, how='inner', on='o')
        intersected_relatedTo = intersected_relatedTo.drop_duplicates().reset_index(drop=True)
        accuracy += round(100 * intersected_relatedTo.shape[0] / relation.shape[0])
        count += 1
    accuracy = accuracy / count
    return accuracy

def plot_accuracy(dicc_acc):


    # set width of bar
    barWidth = 0.1
    # set height of bar (TransD_th85)
    bars1 = dicc_acc['SemEP']
    bars2 = dicc_acc['METIS']
    bars3 = dicc_acc['KMeans']

    # Set position of bar on X axis
    # r1 = np.arange(len(bars1))
    r1 = [0, 0.35, 0.7, 1.05]
    r2 = [x + barWidth for x in r1]
    r3 = [x + barWidth for x in r2]
    r4 = [x + barWidth for x in r3]

    # Make the plot
    plt.bar(r1, bars1, color='#13505b', width=barWidth, edgecolor='white', label='SemEP')
    plt.bar(r2, bars2, color='#90bfc9', width=barWidth, edgecolor='white', label='METIS')
    plt.bar(r3, bars3, color='#2d7f5e', width=barWidth, edgecolor='white', label='KMeans')

    # Add xticks on the middle of the group bars
    plt.ylabel('Accuracy', fontweight='bold')
    plt.ylim(0, 100)

    plt.xticks([0.1, 0.45, 0.8, 1.15], ['TransD', 'TransE', 'TransH', 'TransR'])
    # plt.title('Accuracy of related standards in each cluster')
    """
    for i, v in enumerate(bars1):
        plt.text(r1[i]-0.02, v+0.1, str(v), color='black', fontweight='bold', fontsize='x-small')
    for i, v in enumerate(bars2):
        plt.text(r2[i]-0.02, v+0.1, str(v), color='black', fontweight='bold', fontsize='x-small')
    for i, v in enumerate(bars3):
        plt.text(r3[i]-0.02, v+0.01, str(v), color='black', fontweight='bold', fontsize='x-small')
    """
    # Create legend & Show graphic
    legend = plt.legend(loc='upper right', shadow=False, fontsize='small', ncol=1)

    plt.savefig("accuracy/Accuracy_of_related_standards.pdf", format='pdf', bbox_inches='tight')
    plt.show()

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

list_test_set = ['test_set/test_set_relatedTo0.nt', 'test_set/test_set_relatedTo1.nt', 'test_set/test_set_relatedTo2.nt',
                 'test_set/test_set_relatedTo3.nt', 'test_set/test_set_relatedTo4.nt']
k = 5
dicc_acc = {}
dicc_acc['SemEP'] = []
dicc_acc['KMeans'] = []
dicc_acc['METIS'] = []
for key, address_embedding in list_embedding.items():
    accuracy_semep = 0
    accuracy_kmeans = 0
    accuracy_metis= 0
    for fold in range(k):
        cls_addres = cls_measure + 'SemEP' + str(key) + str(fold) + '/'
        cls_addres_km = cls_measure + 'KMeans' + str(key) + str(fold) + '/'
        cls_addres_metis = cls_measure + 'METIS' + str(key) + str(fold) + '/'
        onlyfiles = [os.path.join(cls_addres + 'clusters/', f) for f in listdir(cls_addres + 'clusters/') if
                     isfile(join(cls_addres + 'clusters/', f))]
        num_cls = len(onlyfiles)

        clusters_semep = load_cluster(cls_addres, num_cls)
        clusters_kmeans = load_cluster(cls_addres_km, num_cls)
        clusters_metis = load_cluster(cls_addres_metis, num_cls)
        test_set, subject = load_test_set(list_test_set[fold])

        accuracy_semep += compute_accuracy(clusters_semep, test_set, subject)
        accuracy_kmeans += compute_accuracy(clusters_kmeans, test_set, subject)
        accuracy_metis += compute_accuracy(clusters_metis, test_set, subject)

    accuracy_semep = round(accuracy_semep / k, 2)
    accuracy_kmeans = round(accuracy_kmeans / k, 2)
    accuracy_metis = round(accuracy_metis / k, 2)

    dicc_acc['SemEP'].append(accuracy_semep)
    dicc_acc['KMeans'].append(accuracy_kmeans)
    dicc_acc['METIS'].append(accuracy_metis)

with open('accuracy/SemEP.txt', "w") as sem:
    sem.write(str(dicc_acc['SemEP']) + "\n")
with open('accuracy/KMeans.txt', "w") as sem:
    sem.write(str(dicc_acc['KMeans']) + "\n")
with open('accuracy/METIS.txt', "w") as sem:
    sem.write(str(dicc_acc['METIS']) + "\n")
plot_accuracy(dicc_acc)