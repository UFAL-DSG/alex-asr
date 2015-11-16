# encoding: utf-8
# distutils: language = c++
from __future__ import unicode_literals

from cython cimport address
from libc.stdlib cimport malloc, free
from libcpp.vector cimport vector
from libcpp cimport bool
from libcpp.string cimport string
cimport fst._fst
cimport fst.libfst
import fst
from kaldi2.utils import lattice_to_nbest


cdef extern from "pykaldi2_decoder/pykaldi2_decoder.h" namespace "kaldi":
    cdef cppclass PyKaldi2Decoder:
        PyKaldi2Decoder(string model_path) except +
        size_t Decode(int max_frames) except +
        void FrameIn(unsigned char *frame, size_t frame_len) except +
        bool GetBestPath(vector[int] *v_out, float *lik) except +
        bool GetLattice(fst.libfst.LogVectorFst *fst_out, double *tot_lik) except +
        string GetWord(int word_id) except +
        void InputFinished() except +
        bool EndpointDetected() except +
        void FinalizeDecoding() except +
        void Reset() except +
        float FinalRelativeCost() except +
        int NumFramesDecoded() except +
        int TrailingSilenceLength() except +
        void GetIvector(vector[float] *ivector) except +


# NOTE: Function signatures as the first line of the docstring are needed in order for
# sphinx to generate nice documentation.
cdef class Decoder:
    """Speech recognition decoder."""

    cdef PyKaldi2Decoder * thisptr
    cdef long fs
    cdef int nchan, bits
    cdef utt_decoded

    def __init__(self, model_path, fs=16000, nchan=1, bits=16):
        """__init__(self, model_path, fs=16000, nchan=1, bits=16)
        Initialise recognizer with audio input stream parameters.

        Args:
            model_path (str): Path where the speech recognition models are stored.
        """
        self.thisptr = new PyKaldi2Decoder(model_path)
        self.utt_decoded = 0
        self.fs, self.nchan, self.bits = fs, nchan, bits
        assert(self.bits % 8 == 0)

    def __dealloc__(self):
        del self.thisptr

    def decode(self, max_frames=10):
        """decode(self, max_frames=10)
        Proceed with decoding the audio.

        Args:
            max_frames (int): Maximum number of frames to decode.

        Returns:
            Number of decoded frames.
        """
        new_dec = self.thisptr.Decode(max_frames)
        self.utt_decoded += new_dec
        return new_dec

    def accept_audio(self, bytes frame_str):
        """accept_audio(self, bytes frame_str)
        Insert given buffer of audio to the decoder for decoding.

        The buffer is interpreted according to the `bits` configuration parameter of the loaded model.
        Usually `bits=16` therefore bytes is interpreted as an array of 16bit little-endian signed integers.

        Args:
            frame_str (bytes): Audio data.
        """
        num_bytes = (self.bits / 8)
        num_samples = len(frame_str) / num_bytes
        assert(num_samples * num_bytes == len(frame_str)), "Not align audio to for %d bits" % self.bits
        self.thisptr.FrameIn(frame_str, num_samples)

    def get_best_path(self):
        """get_best_path(self)
        Get current 1-best decoding hypothesis.

        Returns:
            tuple: (hypothesis likelihood, list of word id's)
        """
        cdef vector[int] t
        cdef float lik
        self.thisptr.GetBestPath(address(t), address(lik))
        words = [t[i] for i in xrange(t.size())]
        return (lik, words)

    def get_nbest(self, n=1):
        """get_nbest(self, n=1)
        Get n best decoding hypotheses (from word posterior lattice).

        Args:
            n (int): How many hypotheses to generate.

        Returns:
            list of hypotheses; each hypothesis is a tuple (hypothesis probability, list of word ids)
        """
        lik, lat = self.get_lattice()
        return lattice_to_nbest(lat, n)

    def get_lattice(self):
        """get_lattice(self)
        Get word posterior lattice and its likelihood.

        NOTE: It may last 100 ms so consideration is needed when used in a timing-critical applications.

        Returns:
            tuple: (lattice likelihood, lattice)

        """
        cdef double lik = -1
        r = fst.LogVectorFst()
        if self.utt_decoded > 0:
            self.thisptr.GetLattice((<fst._fst.LogVectorFst?>r).fst, address(lik))
        self.utt_decoded = 0
        return (lik, r)

    def get_word(self, word_id):
        """get_word(self, word_id)
        Get word string form given word id.

        Args:
            word_id (int): Word id (e.g. as returned by get_best_path).

        Returns:
            Word string (str).
        """
        return self.thisptr.GetWord(word_id)

    def endpoint_detected(self):
        """endpoint_detected(self)
        Has an endpoint been detected?

        Configuration of endpointing is loaded from the model.

        Returns:
            bool whether endpoint was detected
        """
        return self.thisptr.EndpointDetected()

    def get_trailing_silence_length(self):
        """get_trailing_silence_length(self)
        Get number of consecutive silence frames from the end of utterance.

        Returns:
            int number of frames in the best hypothesis, for which silence
            was consecutively decoded, from the end of utterance
        """
        return self.thisptr.TrailingSilenceLength()

    def input_finished(self):
        """input_finished(self)
        Signalize to the decoder that no more input will be added."""
        self.thisptr.InputFinished()

    def finalize_decoding(self):
        """finalize_decoding(self)
        Finalize the decoding and prepare the internal representation for lattice extration."""
        self.thisptr.FinalizeDecoding()

    def reset(self):
        """reset(self)
        Reset the decoder for decoding a new utterance."""
        self.thisptr.Reset()

    def get_final_relative_cost(self):
        """get_final_relative_cost(self)
        Get the relative cost of the decoding so far of the final states.

        Returns:
            float cost
        """
        return self.thisptr.FinalRelativeCost()

    def get_num_frames_decoded(self):
        """get_num_frames_decoded(self)
        Get number of frames decoded so far.

        Returns:
            int number of frames decoded
        """
        return self.thisptr.NumFramesDecoded()

    def get_ivector(self):
        """get_ivector(self)
        Get Ivector of the latest decoded frame.

        Ivector extraction is specified in the model configuration. If the model does not use Ivectors
        this function does not return anything.

        Returns:
            list of floats with the ivector
        """
        cdef vector[float] ivec
        self.thisptr.GetIvector(address(ivec))

        ivector = [ivec[i] for i in xrange(ivec.size())]

        return ivector
