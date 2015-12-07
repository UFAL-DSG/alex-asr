from alex_asr import Decoder
import wave
import struct
import os


MODEL_PATH = "asr_model_digits"

if __name__ == "__main__":
    decoder = Decoder(MODEL_PATH)

    file_name = os.path.join(os.path.dirname(__file__), 'eleven.wav')

    data = wave.open(file_name)

    n_decoded = 0
    while True:
        frames = data.readframes(8000)
        if len(frames) == 0:
            break

        decoder.accept_audio(frames)
        n_decoded += decoder.decode(8000)

        if n_decoded > 0:
            prob, word_ids = decoder.get_best_path()
            # ivec = decoder.get_ivector()
            print 'Hypothesis: "%s" (speaker finished speaking: %s)' % (" ".join(map(decoder.get_word, word_ids)), decoder.endpoint_detected(), )

    decoder.input_finished()
    print 'Final hypothesis:', " ".join(map(decoder.get_word, word_ids))

    decoder.finalize_decoding()

    p, lat = decoder.get_lattice()

    print 'Resulting lattice:'
    for state in lat.states:
        print '  State', state
        for arc in state.arcs:
            print '    ', decoder.get_word(arc.ilabel)



