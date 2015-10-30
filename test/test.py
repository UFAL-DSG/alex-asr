from kaldi2 import decoders
import wave
import struct
import os

MODEL_PATH = "../_models/nnet_model"

if __name__ == "__main__":
    x = decoders.cPyKaldi2Decoder()
    x.setup([
        '--model=%s/final.mdl' % MODEL_PATH,
        '--words=%s/words.txt' % MODEL_PATH,
        '--hclg=%s/HCLG.fst' % MODEL_PATH,
        '--mat_lda=%s/final.mat' % MODEL_PATH,
        '--mat_fmllr=%s/trans.sum' % MODEL_PATH,
        '--mat_cmvn=%s/cmvn_global.mat' % MODEL_PATH,
        '--max-active=7000',
        '--min-active=200',
        '--beam=15.0',
        '--lattice-beam=8.0',
        '--acoustic-scale=0.1',
        '--endpoint.silence-phones=1:2:3:4:5:6:7:8:9:10'
    ])

    data = wave.open(os.path.join(os.path.dirname(__file__), 'test.wav'))
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
            print x.endpoint_detected(), map(x.get_word, word_ids)

    end_time = time.time()

    print x.get_nbest(10)

    dec_duration = end_time - beg_time

    print 'wav duration', wav_duration
    print 'decoding duration', dec_duration

    print 'rtf', wav_duration / dec_duration
