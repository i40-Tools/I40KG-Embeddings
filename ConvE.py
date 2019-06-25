import logging
import os
import sys
import time

import numpy as np
import matplotlib.pyplot as plt
import pykeen
from pykeen.kge_models import ConvE

#%matplotlib inline

logging.basicConfig(level=logging.INFO)
logging.getLogger('pykeen').setLevel(logging.INFO)

print(sys.version)
print(time.asctime())
print(pykeen.get_version())

output_directory = './embeddings/ConvE'

config = dict(
    training_set_path           = '../../tests/resources/data/rdf.nt',
    execution_mode              = 'Training_mode',
    random_seed                 = 0,
    kg_embedding_model_name     = 'ConvE',
    embedding_dim               = 50,
    ConvE_input_channels        = 1,
    ConvE_output_channels       = 3,
    ConvE_height                = 5,
    ConvE_width                 = 10,
    ConvE_kernel_height         = 5,
    ConvE_kernel_width          = 3,
    conv_e_input_dropout        = 0.2,
    conv_e_feature_map_dropout  = 0.5,
    conv_e_output_dropout       = 0.5,
    margin_loss                 = 1,
    learning_rate               = 0.01,
    num_epochs                  = 20,
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
