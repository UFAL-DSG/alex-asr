#include "base/kaldi-types.h"

class PyKaldi2Decoder;

namespace kaldi {
    template<typename > class VectorBase;
}

using namespace kaldi;

PyKaldi2Decoder* instantiatePyKaldi2Decoder();


class PyKaldi2DecoderWrapper {
public:
    PyKaldi2DecoderWrapper()
    {
        decoder = instantiatePyKaldi2Decoder();
    }

    ~PyKaldi2DecoderWrapper() {
        delete decoder;
    };

    size_t Decode(size_t max_frames) {
        return decoder->Decode(max_frames);
    }

    void FrameIn(unsigned char *frame, size_t frame_len) {
        decoder->FrameIn(frame, frame_len);
    }

    void FrameIn(VectorBase<BaseFloat> *waveform_in) {
        decoder->FrameIn(waveform_in);
    }

    bool GetBestPath(std::vector<int> *v_out, BaseFloat *prob) {
        return decoder->GetBestPath(v_out, prob);
    }

    bool EndpointDetected() {
        return decoder->EndpointDetected();
    }

    bool GetLattice(fst::VectorFst<fst::LogArc> * out_fst, double *tot_lik, bool end_of_utt=true) {
        return decoder->GetLattice(out_fst, tot_lik, end_of_utt);
    }

    void FinalizeDecoding() {
        decoder->FinalizeDecoding();
    }

    void Reset(bool reset_pipeline) {
        decoder->Reset(reset_pipeline);
    }

    bool Setup(int argc, const char* const* argv) {
        decoder->Setup(argc, argv);
    }
private:
    PyKaldi2Decoder* decoder;
};