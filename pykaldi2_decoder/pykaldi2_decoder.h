#ifndef PYKALDI2_DECODER_H_
#define PYKALDI2_DECODER_H_
#include <vector>
#include "fst/fst-decl.h"
#include "base/kaldi-types.h"

#ifndef NO_KALDI_HEADERS
#include "pykaldi2_decoder/pykaldi2_decoder_config.h"

#include "feat/online-feature.h"
#include "matrix/matrix-lib.h"
#include "util/common-utils.h"
#include "nnet2/online-nnet2-decodable.h"
#include "online2/online-endpoint.h"

#else
namespace kaldi{
    template <typename Feat> class OnlineGenericBaseFeature;
    class Mfcc;
    class OnlineCmvn;
    class OnlineCmvnState;
    class OnlineSpliceFrames;
    class OnlineTransform;
    typedef OnlineGenericBaseFeature<Mfcc> OnlineMfcc;  // Instance of template for Mfcc/PLP/FilterBanks
    template <typename Num> class Matrix;

    class TransitionModel;
    namespace nnet2 {
      class DecodableNnet2Online;
      struct DecodableNnet2OnlineOptions;
      class AmNnet;
    }
    class LatticeFasterOnlineDecoder;
    struct LatticeFasterDecoderConfig;
    struct OnlineLatgenRecogniserConfig;
    template<typename > class VectorBase;
}

class PyKaldi2DecoderConfig;
#endif



namespace kaldi {

/// \addtogroup online_latgen 
/// @{

    class PyKaldi2Decoder {
    public:
        PyKaldi2Decoder():
                mfcc_(NULL),
                cmvn_(NULL),
                cmvn_state_(NULL),
                splice_(NULL),
                transform_lda_(NULL),
                transform_fmllr_(NULL),
                hclg_(NULL),
                decodable_(NULL),
                decoder_(NULL),
                trans_model_(NULL),
                am_(NULL),
                lda_mat_(NULL),
                fmllr_mat_(NULL),
                cmvn_mat_(NULL),
                config_(NULL)
        { }

        ~PyKaldi2Decoder();
        size_t Decode(size_t max_frames);
        void FrameIn(unsigned char *frame, size_t frame_len);
        void FrameIn(VectorBase<BaseFloat> *waveform_in);
        bool GetBestPath(std::vector<int> *v_out, BaseFloat *prob);
        bool GetLattice(fst::VectorFst<fst::LogArc> * out_fst, double *tot_lik, bool end_of_utt=true);
        string GetWord(int word_id);
        bool EndpointDetected();
        void FinalizeDecoding();
        void Reset(bool reset_pipeline);
        bool Setup(int argc, const char* const* argv);
    private:
        OnlineMfcc *mfcc_;
        OnlineCmvn *cmvn_;
        OnlineCmvnState *cmvn_state_;
        OnlineSpliceFrames *splice_;
        OnlineTransform *transform_lda_;
        OnlineTransform *transform_fmllr_;

        fst::StdFst *hclg_;
        nnet2::DecodableNnet2Online *decodable_;
        LatticeFasterOnlineDecoder *decoder_;

        TransitionModel *trans_model_;
        nnet2::AmNnet *am_;

        Matrix<BaseFloat> *lda_mat_;
        Matrix<BaseFloat> *fmllr_mat_;
        Matrix<double> *cmvn_mat_;

        fst::SymbolTable *words_;

        PyKaldi2DecoderConfig *config_;
        bool initialized_;
        void InitPipeline();
        void InitTransformMatrices();
        void InitDecoder();
        bool ParseConfig(int argc, const char* const* argv);
        void DestroyPipeline();
        void Deallocate();
    };

/// @} end of "addtogroup online_latgen"

} // namespace kaldi

#endif  // #ifdef PYKALDI2_DECODER_H_
