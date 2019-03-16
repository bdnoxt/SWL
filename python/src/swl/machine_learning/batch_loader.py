import os, abc, csv
import numpy as np

#%%------------------------------------------------------------------
# FileBatchLoader.
#	Loads batches from files.
class FileBatchLoader(abc.ABC):
	def __init__(self):
		super().__init__()

	# Returns a generator.
	@abc.abstractmethod
	def loadBatches(self, dir_path, *args, **kwargs):
		raise NotImplementedError

#%%------------------------------------------------------------------
# NpzFileBatchLoader.
#	Loads batches from npz files.
class NpzFileBatchLoader(FileBatchLoader):
	def __init__(self, batch_info_csv_filename=None, data_processing_functor=None):
		super().__init__()

		self._batch_info_csv_filename = 'batch_info.csv' if batch_info_csv_filename is None else batch_info_csv_filename
		self._data_processing_functor = data_processing_functor

	def loadBatches(self, dir_path, *args, **kwargs):
		with open(os.path.join(dir_path, self._batch_info_csv_filename), 'r', encoding='UTF8') as csvfile:
			reader = csv.reader(csvfile)
			for row in reader:
				if not row:
					continue
				try:
					batch_inputs_npzfile = np.load(row[0])
					batch_outputs_npzfile = np.load(row[1])
				except (IOError, ValueError) as ex:
					continue
				num_all_examples = int(row[2])

				for ki, ko in zip(sorted(batch_inputs_npzfile.keys()), sorted(batch_outputs_npzfile.keys())):
					if ki != ko:
						print('Unmatched batch key name: {} != {}.'.format(ki, ko))
						continue
					batch_inputs, batch_outputs = batch_inputs_npzfile[ki], batch_outputs_npzfile[ko]
					num_examples = len(batch_inputs) if len(batch_inputs) == len(batch_outputs) else -1
					if self._data_processing_functor:
						batch_inputs, batch_outputs = self._data_processing_functor(batch_inputs, batch_outputs)
					yield (batch_inputs, batch_outputs), num_examples
