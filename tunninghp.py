import logging
import os
import sys
import time

import numpy as np
import matplotlib.pyplot as plt
import pykeen
from pykeen.kge_models import TransE

%matplotlib inline

logging.basicConfig(level=logging.INFO)
logging.getLogger('pykeen').setLevel(logging.INFO)

print(sys.version)

print(time.asctime())

print(pykeen.get_version())

TransE.hyper_params

output_directory = os.path.join(
    os.path.expanduser('~'),
    'Desktop',
    'pykeen_test'
)

config = dict(
    training_set_path           = './sto/sto.nt',
    test_set_path               = './sto/sto.nt', # Just for illustration, we use the training set also as test set
    execution_mode              = 'HPO_mode', # Define hyper-parameter optimization mode
    random_seed                 = 0,
    kg_embedding_model_name     = 'TransE',
    embedding_dim               = [10,20,50,100],
    scoring_function            = [1,2],
    normalization_of_entities   = [2],  # corresponds to L2
    margin_loss                 = [1,2,3,4,5,6,7,8,9,10],
    learning_rate               = [0.1,0.01,0.001],
    num_epochs                  = [50,100],
    batch_size                  = [32],
    filter_negative_triples     = True,
    maximum_number_of_hpo_iters = 50, # Define the number HPO iterations
    preferred_device            = 'gpu'
)

results = pykeen.run(
    config=config,
    output_directory=output_directory,
)

results.results.keys()

results.results['final_configuration']

results.results['trained_model']

losses = results.results['losses']
epochs = np.arange(len(losses))
plt.title(r'Loss Per Epoch')
plt.xlabel('epoch')
plt.ylabel('loss')
plt.plot(epochs, losses)
plt.show()

results.results['eval_summary']
