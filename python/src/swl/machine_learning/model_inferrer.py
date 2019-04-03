import abc, time
import tensorflow as tf

#%%------------------------------------------------------------------
# ModelInferrer.

#class ModelInferrer(abc.ABC):
class ModelInferrer(object):
	def __init__(self, model, model_save_dir_path):
		super().__init__()

		self._model = model
		self._model_save_dir_path = model_save_dir_path

		# Creates a saver.
		self._saver = tf.train.Saver()

	def infer(self, session, inputs):
		if inputs is None or len(inputs) <= 0:
			print('[SWL] Error: No inference data.')
			return

		if self._saver is not None and self._model_save_dir_path is not None:
			# Load a model.
			# REF [site] >> https://www.tensorflow.org/programmers_guide/saved_model
			# REF [site] >> http://cv-tricks.com/tensorflow-tutorial/save-restore-tensorflow-models-quick-complete-tutorial/
			ckpt = tf.train.get_checkpoint_state(self._model_save_dir_path)
			self._saver.restore(session, ckpt.model_checkpoint_path)
			#self._saver.restore(session, tf.train.latest_checkpoint(self._model_save_dir_path))
			print('[SWL] Info: Loaded a model.')

		print('[SWL] Info: Start inferring...')
		start_time = time.time()
		#inferences = self._model.model_output.eval(session=session, feed_dict=self._model.get_feed_dict((inputs,), is_training=False))
		inferences = session.run(self._model.model_output, feed_dict=self._model.get_feed_dict((inputs,), is_training=False))
		print('\tInference time = {}'.format(time.time() - start_time))
		print('[SWL] Info: End inferring...')

		return inferences