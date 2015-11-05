//
// Created by zilka on 11/5/15.
//

#ifndef PYKALDI_PYKALDI2_FEATURE_PIPELINE_CC_H
#define PYKALDI_PYKALDI2_FEATURE_PIPELINE_CC_H

#include "pykaldi2_decoder/pykaldi2_decoder_config.h"

namespace kaldi {
    class PyKaldi2FeaturePipeline {
    public:
        PyKaldi2FeaturePipeline(PyKaldi2DecoderConfig& config);
        ~PyKaldi2FeaturePipeline();
        OnlineFeatureInterface *GetFeature();
        void AcceptWaveform(BaseFloat sampling_rate,
                            const VectorBase<BaseFloat> &waveform);
        void InputFinished();
    private:
        OnlineMfcc *mfcc_;
        OnlineCmvn *cmvn_;
        OnlineCmvnState *cmvn_state_;
        OnlineSpliceFrames *splice_;
        OnlineTransform *transform_lda_;
        OnlineIvectorFeature *ivector_;
        OnlineAppendFeature *ivector_append_;
        OnlinePitchFeature *pitch_;
        OnlineProcessPitch *pitch_feature_;
        OnlineAppendFeature *pitch_append_;

        OnlineFeatureInterface *final_feature_;
    };
}

#endif //PYKALDI_PYKALDI2_FEATURE_PIPELINE_CC_H
