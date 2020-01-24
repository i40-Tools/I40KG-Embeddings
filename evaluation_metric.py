import pandas as pd
import os
from os.path import isfile, join
from os import listdir
import numpy as np
import matplotlib.pyplot as plt


def get_measure_values(cls_addres, cls_measures, n_cls, metric):
    measure = []
    index_start = static + n_cls -1
    index_end = index_start + n_metric
    for pos in range(index_start, index_end):
        a = cls_measures.iloc[pos].to_string()
        b = a.split('\\t')[1]
        measure.append(float(b))
    cosine_matrix = pd.read_csv(cls_addres + 'cosine_matrix.csv', delimiter=",")
    max_cut = sum(cosine_matrix.sum(axis=0, skipna=True))
    measure[0] = 1.0 - measure[0]
    measure[2] = (measure[2] + 0.5) / 1.5
    measure[3] = 1 - (measure[3] / max_cut)
    alg = [measure[0], measure[4], measure[3], measure[2], measure[1]]
    alg = [round(i * 10, 2) for i in alg]
    metric = [x + y for x, y in zip(metric, alg)]
    return metric


def radar_plot(metric_semep, metric_met, metric_km, key):
    # Optionally use different styles for the graph
    # Gallery: http://tonysyu.github.io/raw_content/matplotlib-style-gallery/gallery.html
    # import matplotlib
    # matplotlib.style.use('dark_background')  # interesting: 'bmh' / 'ggplot' / 'dark_background'

    class Radar(object):
        def __init__(self, figure, title, labels, rect=None):
            if rect is None:
                rect = [0.05, 0.05, 0.9, 0.9]

            self.n = len(title)
            self.angles = np.arange(0, 360, 360.0 / self.n)

            self.axes = [figure.add_axes(rect, projection='polar', label='axes%d' % i) for i in range(self.n)]

            self.ax = self.axes[0]
            self.ax.set_thetagrids(self.angles, labels=title, fontsize=14)

            for ax in self.axes[1:]:
                ax.patch.set_visible(False)
                ax.grid(False)
                ax.xaxis.set_visible(False)

            for ax, angle, label in zip(self.axes, self.angles, labels):
                ax.set_rgrids(range(0, 12), angle=angle, labels=label)
                ax.spines['polar'].set_visible(False)
                ax.set_ylim(0, 10)

        def plot(self, values, *args, **kw):
            angle = np.deg2rad(np.r_[self.angles, self.angles[0]])
            values = np.r_[values, values[0]]
            self.ax.plot(angle, values, *args, **kw)
            self.ax.fill(angle, values, 'r', alpha=0.1)

    if __name__ == '__main__':
        fig = plt.figure(figsize=(5, 5))

        tit = ['C', 'P', 'T', 'M', 'Co']  # 12x

        lab = [
            ['0', '0.1', '0.2', '0.3', '0.4', '0.5', '0.6', '0.7', '0.8', '0.9'],
            ['0', '0.1', '0.2', '0.3', '0.4', '0.5', '0.6', '0.7', '0.8', '0.9'],
            ['0', '0.1', '0.2', '0.3', '0.4', '0.5', '0.6', '0.7', '0.8', '0.9'],
            ['0', '0.1', '0.2', '0.3', '0.4', '0.5', '0.6', '0.7', '0.8', '0.9'],
            ['0', '0.1', '0.2', '0.3', '0.4', '0.5', '0.6', '0.7', '0.8', '0.9']
        ]

        radar = Radar(fig, tit, lab)
        radar.plot(metric_semep, linestyle='solid', linewidth=2, color='b', alpha=0.7,
                   label='SemEP')
        radar.plot(metric_met, linestyle='solid', linewidth=2, color='r', alpha=0.7,
                   label='METIS')
        radar.plot(metric_km, linestyle='solid', linewidth=2, color='g', alpha=0.7,
                   label='KMeans')

        if key == 'TransD':
            radar.ax.legend(loc=(0.15, 1.04), ncol=3, fontsize='large')
        fig.savefig('evaluation_metric/' + str(key) + '.pdf', format='pdf', bbox_inches='tight')



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
k = 5
static = 16
n_metric = 5
dicc_metric = {}

for key, address_embedding in list_embedding.items():
    metric_semep = [0, 0, 0, 0, 0]
    metric_km = [0, 0, 0, 0, 0]
    metric_met = [0, 0, 0, 0, 0]
    for fold in range(k):

        cls_addres = cls_measure + 'SemEP' + str(key) + str(fold) + '/'
        cls_addres_km = cls_measure + 'KMeans' + str(key) + str(fold) + '/'
        cls_addres_metis = cls_measure + 'METIS' + str(key) + str(fold) + '/'
        folder_semep = 'SemEP' + str(key) + str(fold)
        folder_kmeans = 'KMeans' + str(key) + str(fold)
        folder_metis = 'METIS' + str(key) + str(fold)

        cls_measures_semep = pd.read_csv(cls_addres + folder_semep+'.txt', delimiter=",")
        cls_measures_km = pd.read_csv(cls_addres_km + folder_kmeans + '.txt', delimiter=",")
        cls_measures_met = pd.read_csv(cls_addres_metis + folder_metis + '.txt', delimiter=",")
        onlyfiles = [os.path.join(cls_addres + 'clusters/', f) for f in listdir(cls_addres + 'clusters/') if
                     isfile(join(cls_addres + 'clusters/', f))]
        num_cls = len(onlyfiles)

        metric_semep = get_measure_values(cls_addres, cls_measures_semep, num_cls, metric_semep)
        metric_km = get_measure_values(cls_addres_km, cls_measures_km, num_cls, metric_km)
        metric_met = get_measure_values(cls_addres_metis, cls_measures_met, num_cls, metric_met)


    metric_semep = [round(i / k, 2) for i in metric_semep]
    metric_km = [round(i / k, 2) for i in metric_km]
    metric_met = [round(i / k, 2) for i in metric_met]
    dicc_metric['SemEP' + str(key)] = metric_semep
    dicc_metric['KMeans' + str(key)] = metric_km
    dicc_metric['METIS' + str(key)] = metric_met

    print('====')
    print(dicc_metric['SemEP' + str(key)])
    print(dicc_metric['KMeans' + str(key)])
    print(dicc_metric['METIS' + str(key)])
    print('====')
    with open('evaluation_metric/SemEP' + str(key) + '.txt', "w") as sem:
        sem.write(str(dicc_metric['SemEP' + str(key)]) + "\n")
    with open('evaluation_metric/KMeans' + str(key) + '.txt', "w") as sem:
        sem.write(str(dicc_metric['KMeans' + str(key)]) + "\n")
    with open('evaluation_metric/METIS' + str(key) + '.txt', "w") as sem:
        sem.write(str(dicc_metric['METIS' + str(key)]) + "\n")
    radar_plot(metric_semep, metric_met, metric_km, key)



