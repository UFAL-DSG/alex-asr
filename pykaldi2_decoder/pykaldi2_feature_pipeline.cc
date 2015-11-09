//
// Created by zilka on 11/5/15.
//

#include "pykaldi2_decoder/pykaldi2_feature_pipeline.h"

namespace kaldi {
    PyKaldi2FeaturePipeline::PyKaldi2FeaturePipeline(PyKaldi2DecoderConfig &config) :
        mfcc_(NULL),
        cmvn_(NULL),
        cmvn_state_(NULL),
        splice_(NULL),
        transform_lda_(NULL),
        ivector_(NULL),
        ivector_append_(NULL),
        pitch_(NULL),
        pitch_feature_(NULL),
        pitch_append_(NULL),
        final_feature_(NULL)

    {
        OnlineFeatureInterface *prev_feature;

        KALDI_VLOG(3) << "Feature MFCC "
                      << config.mfcc_opts.mel_opts.low_freq
                      << " " << config.mfcc_opts.mel_opts.high_freq;
        prev_feature = mfcc_ = new OnlineMfcc(config.mfcc_opts);
        KALDI_VLOG(3) << "    -> dims: " << mfcc_->Dim();

        if (config.use_cmvn) {
            KALDI_VLOG(3) << "Feature CMVN";
            cmvn_state_ = new OnlineCmvnState(*config.cmvn_mat);
            prev_feature = cmvn_ = new OnlineCmvn(config.cmvn_opts, *cmvn_state_, prev_feature);
        }
        KALDI_VLOG(3) << "Feature SPLICE " << config.splice_opts.left_context << " " <<
                      config.splice_opts.right_context;
        prev_feature = splice_ = new OnlineSpliceFrames(config.splice_opts, prev_feature);
        KALDI_VLOG(3) << "    -> dims: " << splice_->Dim();

        KALDI_VLOG(3) << "Feature LDA " << config.lda_mat->NumRows() << " " << config.lda_mat->NumCols();
        prev_feature = transform_lda_ = new OnlineTransform(*config.lda_mat, prev_feature);
        KALDI_VLOG(3) << "    -> dims: " << transform_lda_->Dim();

        if (config.use_pitch) {
            pitch_ = new OnlinePitchFeature(config.pitch_opts);
            pitch_feature_ = new OnlineProcessPitch(config.pitch_process_opts, pitch_);
            pitch_append_ = new OnlineAppendFeature(mfcc_, pitch_feature_);
        }

        if (config.use_ivectors) {
            KALDI_VLOG(3) << "Feature IVectors";
            ivector_ = new OnlineIvectorFeature(*config.ivector_extraction_info, mfcc_);
            prev_feature = ivector_append_ = new OnlineAppendFeature(mfcc_, ivector_);
            KALDI_VLOG(3) << "     -> dims: " << prev_feature->Dim();
        }

        final_feature_ = prev_feature;
    }

    PyKaldi2FeaturePipeline::~PyKaldi2FeaturePipeline() {
        delete mfcc_;
        delete cmvn_;
        delete cmvn_state_;
        delete splice_;
        delete transform_lda_;
        delete ivector_;
        delete ivector_append_;
        delete pitch_;
        delete pitch_feature_;
        delete pitch_append_;
    }

    OnlineFeatureInterface *PyKaldi2FeaturePipeline::GetFeature() {
        return final_feature_;
    }

    void PyKaldi2FeaturePipeline::AcceptWaveform(BaseFloat sampling_rate,
                                                 const VectorBase<BaseFloat> &waveform) {
        mfcc_->AcceptWaveform(sampling_rate, waveform);
    }

    void PyKaldi2FeaturePipeline::InputFinished() {
        mfcc_->InputFinished();
    }

    OnlineIvectorFeature *PyKaldi2FeaturePipeline::GetIvectorFeature() {
        return ivector_;
    }
}

