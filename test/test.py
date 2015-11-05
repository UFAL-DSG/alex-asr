from kaldi2 import decoders
import wave
import struct
import os

MODEL_PATH = "../_models/nnet_model_cs_voip"

if __name__ == "__main__":
    x = decoders.cPyKaldi2Decoder(MODEL_PATH)

    print os.getcwd()

    #data = wave.open(os.path.join(os.path.dirname(__file__), 'test2.wav'))
    data = wave.open('test/test2.wav')
    wav_duration = data.getnframes() * 1.0 / data.getframerate()

    import time

    beg_time = time.time()
    n_decoded = 0
    while True:
        frames = data.readframes(8000)
        if len(frames) == 0:
            break
        x.frame_in(frames)
        n_decoded += x.decode(8000)

        if n_decoded > 0:
            prob, word_ids = x.get_best_path()
            print map(x.get_word, word_ids)  #x.endpoint_detected(),

    end_time = time.time()

    print x.get_nbest(10)

    dec_duration = end_time - beg_time

    print 'wav duration', wav_duration
    print 'decoding duration', dec_duration

    print 'rtf', wav_duration / dec_duration
