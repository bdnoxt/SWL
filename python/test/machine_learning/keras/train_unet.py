# Path to libcudnn.so.5.
#export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH

#%%------------------------------------------------------------------

import os
if 'posix' == os.name:
	swl_python_home_dir_path = '/home/sangwook/work/SWL_github/python'
else:
	swl_python_home_dir_path = 'D:/work/SWL_github/python'
os.chdir(swl_python_home_dir_path + '/test/machine_learning/keras')

import sys
sys.path.append('../../../src')

#%%------------------------------------------------------------------

import numpy as np
import tensorflow as tf
import keras
from keras import backend as K
from keras.preprocessing.image import ImageDataGenerator
from swl.machine_learning.keras.unet import UNet
from swl.machine_learning.keras.loss import dice_coeff, dice_coeff_loss
from swl.machine_learning.data_loader import DataLoader

#%%------------------------------------------------------------------

config = tf.ConfigProto()
#config.allow_soft_placement = True
config.log_device_placement = True
config.gpu_options.allow_growth = True
#config.gpu_options.per_process_gpu_memory_fraction = 0.4  # only allocate 40% of the total memory of each GPU.
sess = tf.Session(config=config)

# This means that Keras will use the session we registered to initialize all variables that it creates internally.
K.set_session(sess)
K.set_learning_phase(0)

#%%------------------------------------------------------------------
# Load data.

#dataset_home_dir_path = "/home/sangwook/my_dataset"
#dataset_home_dir_path = "/home/HDD1/sangwook/my_dataset"
dataset_home_dir_path = "D:/dataset"

train_dataset_dir_path = dataset_home_dir_path + "/biomedical_imaging/isbi2012_em_segmentation_challenge/train"
test_dataset_dir_path = dataset_home_dir_path + "/biomedical_imaging/isbi2012_em_segmentation_challenge/test"

train_summary_dir_path = './train'
test_summary_dir_path = './test'

# NOTICE [caution] >>
#	If the size of data is changed, labels may be dense.

data_loader = DataLoader()
#data_loader = DataLoader(224, 224)
train_dataset = data_loader.load(train_dataset_dir_path, data_suffix='', data_extension='tif', label_suffix='_mask', label_extension='tif')

# Change the dimension of labels.
if train_dataset.data.ndim == train_dataset.labels.ndim:
	pass
elif 1 == train_dataset.data.ndim - train_dataset.labels.ndim:
	train_dataset.labels = train_dataset.labels.reshape(train_dataset.labels.shape + (1,))
else:
	raise ValueError('train_dataset.data.ndim or train_dataset.labels.ndim is invalid.')

# Change labels from grayscale values to indexes.
for train_label in train_dataset.labels:
	unique_lbls = np.unique(train_label).tolist()
	for lbl in unique_lbls:
		train_label[train_label == lbl] = unique_lbls.index(lbl)

#print(np.max(train_dataset.labels))
#print(np.unique(train_dataset.labels).shape[0])
#print(np.max(train_dataset.labels[0]))
#print(np.unique(train_dataset.labels[0]).shape[0])

assert train_dataset.data.shape[0] == train_dataset.labels.shape[0] and train_dataset.data.shape[1] == train_dataset.labels.shape[1] and train_dataset.data.shape[2] == train_dataset.labels.shape[2], "ERROR: Image and label size mismatched."

#%%------------------------------------------------------------------

keras_backend = 'tf'

num_examples = train_dataset.num_examples
num_classes = np.unique(train_dataset.labels).shape[0]  # 2.

batch_size = 1
num_epochs = 50
steps_per_epoch = num_examples // batch_size
if steps_per_epoch < 1:
	steps_per_epoch = 1

resized_input_size = train_dataset.data.shape[1:3]  # (height, width) = (512, 512).

train_data_shape = (None,) + resized_input_size + (train_dataset.data.shape[3],)
train_label_shape = (None,) + resized_input_size + (1 if 2 == num_classes else num_classes,)
train_data_tf = tf.placeholder(tf.float32, shape=train_data_shape)
#train_labels_tf = tf.placeholder(tf.float32, shape=train_label_shape)
train_labels_tf = tf.placeholder(tf.uint8, shape=train_label_shape)

#%%------------------------------------------------------------------
# Create a data generator.

# REF [site] >> https://keras.io/preprocessing/image/

train_data_generator = ImageDataGenerator(
	rescale=1./255.,
	#preprocessing_function=None,
	featurewise_center=True,
	featurewise_std_normalization=True,
	#samplewise_center=False,
	#samplewise_std_normalization=False,
	#zca_whitening=False,
	#zca_epsilon=1e-6,
	rotation_range=20,
	width_shift_range=0.2,
	height_shift_range=0.2,
	horizontal_flip=True,
	vertical_flip=True,
	zoom_range=0.2,
	#shear_range=0.,
	#channel_shift_range=0.,
	fill_mode='reflect',
	cval=0.)
train_label_generator = ImageDataGenerator(
	#rescale=1./255.,
	#preprocessing_function=None,
	#featurewise_center=True,
	#featurewise_std_normalization=True,
	#samplewise_center=False,
	#samplewise_std_normalization=False,
	#zca_whitening=False,
	#zca_epsilon=1e-6,
	rotation_range=20,
	width_shift_range=0.2,
	height_shift_range=0.2,
	horizontal_flip=True,
	vertical_flip=True,
	zoom_range=0.2,
	#shear_range=0.,
	#channel_shift_range=0.,
	fill_mode='reflect',
	cval=0.)

# Provide the same seed and keyword arguments to the fit and flow methods.
seed = 1

# Compute the internal data stats related to the data-dependent transformations, based on an array of sample data.
# Only required if featurewise_center or featurewise_std_normalization or zca_whitening.
train_data_generator.fit(train_dataset.data, augment=True, seed=seed)
#train_label_generator.fit(train_dataset.labels, augment=True, seed=seed)

train_data_gen = train_data_generator.flow(train_dataset.data, batch_size=batch_size, shuffle=True, seed=seed)
train_label_gen = train_label_generator.flow(train_dataset.labels, batch_size=batch_size, shuffle=True, seed=seed)

# Combine generators into one which yields image and labels.
train_dataset_gen = zip(train_data_gen, train_label_gen)

#%%------------------------------------------------------------------
# Create a U-Net model.

with tf.name_scope('unet'):
	unet_model = UNet().create_model(num_classes, backend=keras_backend, input_shape=train_data_shape, tf_input=train_data_tf)
#	if 'tf' == keras_backend:
#		Model(inputs=Input(tensor=train_data_tf), outputs=unet_model).summary()
#	else:   
#		unet_model.summary()

#unet_model_output = unet_model(train_data_tf)

#%%------------------------------------------------------------------
# Configure training parameters of the FC-DenseNet model.

# Define a loss.
with tf.name_scope('loss'):
	#loss = tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(labels=train_labels_tf, logits=unet_model))  # NOTICE [caution] >> float required.
	loss = tf.reduce_mean(tf.nn.softmax_cross_entropy_with_logits(labels=train_labels_tf, logits=unet_model))

# Define an optimzer.
global_step = tf.Variable(0, name='global_step', trainable=False)
with tf.name_scope('learning_rate'):
	learning_rate = tf.train.exponential_decay(0.0001, global_step, steps_per_epoch*3, 0.5, staircase=True)
train_step = tf.train.MomentumOptimizer(learning_rate=learning_rate, momentum=0.99).minimize(loss, global_step=global_step)

#%%------------------------------------------------------------------
# Train the U-Net model.

# Merge all the summaries and write them out to a directory.
merged = tf.summary.merge_all()
train_summary_writer = tf.summary.FileWriter(train_summary_dir_path, sess.graph)
test_summary_writer = tf.summary.FileWriter(test_summary_dir_path)

# Saves a model every 2 hours and maximum 5 latest models are saved.
saver = tf.train.Saver(max_to_keep=5, keep_checkpoint_every_n_hours=2)

# Initialize all variables.
sess.run(tf.global_variables_initializer())

# Run training loop.
with sess.as_default():
	for epoch in range(num_epochs):
		print('Epoch %d/%d' % (epoch + 1, num_epochs))
		steps = 0
		for data_batch, label_batch in train_dataset_gen:
			if 2 == num_classes:
				label_batch = label_batch.astype(np.uint8)
			else:
				label_batch = keras.utils.to_categorical(label_batch, num_classes).reshape(label_batch.shape[:-1] + (-1,)).astype(np.uint8)
			#print('data batch: (shape, dtype, min, max) =', data_batch.shape, data_batch.dtype, np.min(data_batch), np.max(data_batch))
			#print('label batch: (shape, dtype, min, max) =', label_batch.shape, label_batch.dtype, np.min(label_batch), np.max(label_batch))
			train_step.run(feed_dict={train_data_tf: data_batch, train_labels_tf: label_batch})
			steps += 1
			if steps >= steps_per_epoch:
				break

#%%------------------------------------------------------------------
# Evaluate the U-Net model.

# Compute dice score for simple evaluation during training.
with tf.name_scope('dice_eval'):
    dice_evaluator = tf.reduce_mean(dice_coeff(train_labels_tf, unet_model))
