import logging
import os
import sys
import time

import numpy as np
import matplotlib.pyplot as plt
import pykeen
from pykeen.kge_models import TransD

#%matplotlib inline

logging.basicConfig(level=logging.INFO)
logging.getLogger('pykeen').setLevel(logging.INFO)

print(sys.version)
print(time.asctime())
print(pykeen.get_version())

output_directory = './embeddings/TransD/sto-enriched'

config = dict(
    training_set_path           = './sto/sto-enriched.nt',
    execution_mode              = 'Training_mode',
    random_seed                 = 0,
    kg_embedding_model_name     = 'TransD',
    embedding_dim               = 50,
    relation_embedding_dim      = 20,
    scoring_function            = 2,  # corresponds to L2
    margin_loss                 = 0.05,
    learning_rate               = 0.01,
    num_epochs                  = 100,
    batch_size                  = 64,
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
