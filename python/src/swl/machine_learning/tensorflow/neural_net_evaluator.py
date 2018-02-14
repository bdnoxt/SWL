import numpy as np

#%%------------------------------------------------------------------

class NeuralNetEvaluator(object):
	def evaluate(self, session, neuralNet, test_data, test_labels, batch_size=None):
		loss, accuracy = neuralNet.loss, neuralNet.accuracy

		num_test_examples = test_data.shape[0]

		if batch_size is None or num_test_examples <= batch_size:
			#test_loss = loss.eval(session=session, feed_dict=neuralNet.get_feed_dict(test_data, test_labels, is_training=False))
			#test_acc = accuracy.eval(session=session, feed_dict=neuralNet.get_feed_dict(test_data, test_labels, is_training=False))
			test_loss, test_acc = session.run([loss, accuracy], feed_dict=neuralNet.get_feed_dict(test_data, test_labels, is_training=False))
		else:
			test_steps_per_epoch = (num_test_examples - 1) // batch_size + 1

			indices = np.arange(num_test_examples)
			#if shuffle:
			#	np.random.shuffle(indices)

			test_loss, test_acc = 0, 0
			for step in range(test_steps_per_epoch):
				start = step * batch_size
				end = start + batch_size
				batch_indices = indices[start:end]
				if batch_indices.size > 0:  # If batch_indices is non-empty.
					data_batch, label_batch = test_data[batch_indices,], test_labels[batch_indices,]
					if data_batch.size > 0 and label_batch.size > 0:  # If data_batch and label_batch are non-empty.
						#batch_loss = loss.eval(session=session, feed_dict=neuralNet.get_feed_dict(data_batch, label_batch, is_training=False))
						#batch_acc = accuracy.eval(session=session, feed_dict=neuralNet.get_feed_dict(data_batch, label_batch, is_training=False))
						batch_loss, batch_acc = session.run([loss, accuracy], feed_dict=neuralNet.get_feed_dict(data_batch, label_batch, is_training=False))
	
						# TODO [check] >> Is test_loss or test_acc correct?
						test_loss += batch_loss * batch_indices.size
						test_acc += batch_acc * batch_indices.size
			test_loss /= num_test_examples
			test_acc /= num_test_examples

		return test_loss, test_acc

	def evaluate_seq2seq(self, session, neuralNet, test_encoder_inputs, test_decoder_inputs, test_decoder_outputs, batch_size=None):
		loss, accuracy = neuralNet.loss, neuralNet.accuracy

		num_test_examples = test_encoder_inputs.shape[0]

		if batch_size is None or num_test_examples <= batch_size:
			#test_loss = loss.eval(session=session, feed_dict=neuralNet.get_feed_dict(test_encoder_inputs, test_decoder_inputs, test_decoder_outputs, is_training=False))
			#test_acc = accuracy.eval(session=session, feed_dict=neuralNet.get_feed_dict(test_encoder_inputs, test_decoder_inputs, test_decoder_outputs, is_training=False))
			test_loss, test_acc = session.run([loss, accuracy], feed_dict=neuralNet.get_feed_dict(test_encoder_inputs, test_decoder_inputs, test_decoder_outputs, is_training=False))
		else:
			test_steps_per_epoch = (num_test_examples - 1) // batch_size + 1

			indices = np.arange(num_test_examples)
			#if shuffle:
			#	np.random.shuffle(indices)

			test_loss, test_acc = 0, 0
			for step in range(test_steps_per_epoch):
				start = step * batch_size
				end = start + batch_size
				batch_indices = indices[start:end]
				if batch_indices.size > 0:  # If batch_indices is non-empty.
					enc_input_batch, dec_input_batch, dec_output_batch = test_encoder_inputs[batch_indices,], test_decoder_inputs[batch_indices,], test_decoder_outputs[batch_indices,]
					if enc_input_batch.size > 0 and dec_input_batch.size > 0 and dec_output_batch.size > 0:  # If enc_input_batch, dec_input_batch, and dec_output_batch are non-empty.
						#batch_loss = loss.eval(session=session, feed_dict=neuralNet.get_feed_dict(enc_input_batch, dec_input_batch, dec_output_batch, is_training=False))
						#batch_acc = accuracy.eval(session=session, feed_dict=neuralNet.get_feed_dict(enc_input_batch, dec_input_batch, dec_output_batch, is_training=False))
						batch_loss, batch_acc = session.run([loss, accuracy], feed_dict=neuralNet.get_feed_dict(enc_input_batch, dec_input_batch, dec_output_batch, is_training=False))
	
						# TODO [check] >> Is test_loss or test_acc correct?
						test_loss += batch_loss * batch_indices.size
						test_acc += batch_acc * batch_indices.size
			test_loss /= num_test_examples
			test_acc /= num_test_examples

		return test_loss, test_acc
