import logging
import os
import sys
import time

import numpy as np
import matplotlib.pyplot as plt
import pykeen

#%matplotlib inline

logging.basicConfig(level=logging.INFO)
logging.getLogger('pykeen').setLevel(logging.INFO)

print(sys.version)
print(time.asctime())
print(pykeen.get_version())

output_directory = './embeddings/TransE/sto-enriched'

config = dict(
    training_set_path           = './sto/sto-enriched.nt',
    execution_mode              = 'Training_mode',
    random_seed                 = 0,
    kg_embedding_model_name     = 'TransE',
    embedding_dim               = 50,
    scoring_function            = 1,  # corresponds to L1
    normalization_of_entities   = 2,  # corresponds to L2
    margin_loss                 = 1,
    learning_rate               = 0.01,
    num_epochs                  = 100,
    batch_size                  = 64,
    test_set_ratio              = 0.1, # 10% of training set will be used as a test set
    filter_negative_triples     = True,
    preferred_device            = 'gpu'
)

print('Starting the training...')
results = pykeen.run(
    config=config,
    output_directory=output_directory,
)
print('Training is finished')

results.results.keys()

results.results['trained_model']

losses = results.results['losses']
epochs = np.arange(len(losses))

#plt.title(r'Loss Per Epoch')
#plt.xlabel('epoch')
#plt.ylabel('loss')
#plt.plot(epochs, losses)
#plt.show()
