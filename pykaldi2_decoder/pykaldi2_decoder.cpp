#include "pykaldi2_decoder.h"
#include "pykaldi2_decoder/utils.h"

#include "online2/onlinebin-util.h"


namespace kaldi {
    bool PyKaldi2Decoder::Setup(const string model_path) {
        initialized_ = false;

        local_cwd cwd_to_model_path(model_path);  // Change dir to model_path. Change back when leaving the scope.

        try {
            KALDI_LOG << "Setup.";
            if(!ParseConfig()) {
                return false;
            }

            InitDecoder();
            InitTransformMatrices();
            Reset();

            initialized_ = true;

            return true;
        } catch(const std::exception& e) {
            Deallocate();

            std::cerr << e.what() << std::endl;
            return false;
        }
    }

    void PyKaldi2Decoder::Reset() {
        InitPipeline();
        decoder_->InitDecoding();
    }

    void PyKaldi2Decoder::InitPipeline() {
        DestroyPipeline();

        OnlineFeatureInterface *prev_feature;

        KALDI_LOG << "Feature MFCC " << config_->mfcc_opts.mel_opts.low_freq << " " << config_->mfcc_opts.mel_opts.high_freq;
        prev_feature = mfcc_ = new OnlineMfcc(config_->mfcc_opts);
        KALDI_LOG << "    -> dims: " << mfcc_->Dim();

        if (config_->use_cmvn) {
            KALDI_LOG << "Feature CMVN";
            cmvn_state_ = new OnlineCmvnState(*cmvn_mat_);
            prev_feature = cmvn_ = new OnlineCmvn(config_->cmvn_opts, *cmvn_state_, prev_feature);
        }
        KALDI_LOG << "Feature SPLICE " << config_->splice_opts.left_context << " " << config_->splice_opts.right_context;
        prev_feature = splice_ = new OnlineSpliceFrames(config_->splice_opts, prev_feature);
        KALDI_LOG << "    -> dims: " << splice_->Dim();

        KALDI_LOG << "Feature LDA " << lda_mat_->NumRows() << " " << lda_mat_->NumCols();
        prev_feature = transform_lda_ = new OnlineTransform(*lda_mat_, prev_feature);
        KALDI_LOG << "    -> dims: " << transform_lda_->Dim();

        if(config_->use_pitch) {
            pitch_ = new OnlinePitchFeature(config_->pitch_opts);
            pitch_feature_ = new OnlineProcessPitch(config_->pitch_process_opts, pitch_);
            pitch_append_ = new OnlineAppendFeature(mfcc_, pitch_feature_);
        }

        if (config_->use_ivectors) {
            KALDI_LOG << "Feature IVectors";
            ivector_ = new OnlineIvectorFeature(*ivector_extraction_info_, mfcc_);
            prev_feature = ivector_append_ = new OnlineAppendFeature(mfcc_, ivector_);
            KALDI_LOG << "     -> dims: " << prev_feature->Dim();
        }

        final_feature_ = prev_feature;

        decodable_ = new nnet2::DecodableNnet2Online(*am_,
                                                     *trans_model_,
                                                     config_->decodable_opts,
                                                     final_feature_);
    }



    bool PyKaldi2Decoder::ParseConfig() {
        if (config_ == NULL)
            config_ = new PyKaldi2DecoderConfig();

        config_->LoadConfigs("pykaldi.cfg");


        if(!config_->Check()) {
            KALDI_ERR << "Please check that you specified all required arguments.";
            return false;
        } else {
            return true;
        }
    }

    void PyKaldi2Decoder::InitDecoder() {
        bool binary;
        Input ki(config_->model_rxfilename, &binary);

        trans_model_ = new TransitionModel();
        trans_model_->Read(ki.Stream(), binary);

        am_ = new nnet2::AmNnet();
        am_->Read(ki.Stream(), binary);

        hclg_ = ReadDecodeGraph(config_->fst_rxfilename);
        decoder_ = new LatticeFasterOnlineDecoder(*hclg_, config_->decoder_opts);

        words_ = fst::SymbolTable::ReadText(config_->words_rxfilename);
    }

    void PyKaldi2Decoder::InitTransformMatrices() {
        {
            KALDI_LOG << "Loading LDA matrix.";
            bool binary_in;
            Input ki(config_->lda_mat_rspecifier, &binary_in);
            delete lda_mat_;
            lda_mat_ = new Matrix<BaseFloat>();
            lda_mat_->Read(ki.Stream(), binary_in);
        }

        if(config_->fcmvn_mat_rspecifier != "")
        {
            KALDI_LOG << "Loading global CMVN stats.";
            bool binary_in;
            Input ki(config_->fcmvn_mat_rspecifier, &binary_in);
            delete cmvn_mat_;
            cmvn_mat_ = new Matrix<double>();
            cmvn_mat_->Read(ki.Stream(), binary_in);
        }
        if(config_->use_ivectors) {
            KALDI_LOG << "Loading IVector extraction info.";
            ivector_extraction_info_ = new OnlineIvectorExtractionInfo(config_->ivector_config);
        }
    }

    bool PyKaldi2Decoder::EndpointDetected() {
        return kaldi::EndpointDetected(config_->endpoint_config, *trans_model_,
                                       config_->mfcc_opts.frame_opts.frame_shift_ms * 1.0e-03,
                                       *decoder_);
    }

    void PyKaldi2Decoder::FrameIn(VectorBase<BaseFloat> *waveform_in) {
        mfcc_->AcceptWaveform(config_->mfcc_opts.frame_opts.samp_freq, *waveform_in);
    }

    void PyKaldi2Decoder::FrameIn(unsigned char *frame, size_t frame_len) {
        if (! initialized_)
            return;

        Vector<BaseFloat> waveform(frame_len);
        for(size_t i = 0; i < frame_len; ++i) {
            switch(config_->bits_per_sample) {
                case 8:
                {
                    waveform(i) = (*frame);
                    frame++;
                    break;
                }
                case 16:
                {
                    int16 k = *reinterpret_cast<uint16*>(frame);
#ifdef __BIG_ENDDIAN__
                    KALDI_SWAP2(k);
#endif
                    waveform(i) = k;
                    frame += 2;
                    break;
                }
                default:
                    KALDI_ERR << "Unsupported bits ber sample (implement yourself): "
                    << config_->bits_per_sample;
            }
        }
        this->FrameIn(&waveform);
    }

    void PyKaldi2Decoder::InputFinished() {
        mfcc_->InputFinished();
    }

    size_t PyKaldi2Decoder::Decode(size_t max_frames) {
        if (! initialized_)
            return 0;
        size_t decoded = decoder_->NumFramesDecoded();
        decoder_->AdvanceDecoding(decodable_, max_frames);
        return decoder_->NumFramesDecoded() - decoded;
    }

    void PyKaldi2Decoder::FinalizeDecoding() {
        decoder_->FinalizeDecoding();
    }

    bool PyKaldi2Decoder::GetBestPath(std::vector<int> *out_words, BaseFloat *prob) {
        *prob = -1.0f;
        if (! initialized_)
            return false;

        Lattice lat;
        bool ok = decoder_->GetBestPath(&lat);

        LatticeWeight weight;
        std::vector<int32> ids;
        fst::GetLinearSymbolSequence(lat,
                                     static_cast<vector<int32> *>(0),
                                     out_words,
                                     &weight);

        *prob = weight.Value1() + weight.Value2();

        return ok;
    }

    bool PyKaldi2Decoder::GetLattice(fst::VectorFst<fst::LogArc> *fst_out,
                                     double *tot_lik, bool end_of_utterance) {
        CompactLattice lat;
        Lattice raw_lat;

        if (! initialized_)
            return false;

        if (decoder_->NumFramesDecoded() == 0)
            KALDI_ERR << "You cannot get a lattice if you decoded no frames.";

        if (!config_->decoder_opts.determinize_lattice)
            KALDI_ERR << "--determinize-lattice=false option is not supported at the moment";

        bool ok = decoder_->GetRawLattice(&raw_lat, end_of_utterance);

        BaseFloat lat_beam = config_->decoder_opts.lattice_beam;
        DeterminizeLatticePhonePrunedWrapper(
                *trans_model_, &raw_lat, lat_beam, &lat, config_->decoder_opts.det_opts);

        *tot_lik = CompactLatticeToWordsPost(lat, fst_out);

        return ok;
    }

    string PyKaldi2Decoder::GetWord(int word_id) {
        return words_->Find(word_id);
    }

    PyKaldi2Decoder::~PyKaldi2Decoder() {
        Deallocate();
    }

    void PyKaldi2Decoder::Deallocate() {
        initialized_ = false;

        DestroyPipeline();

        delete hclg_; hclg_ = NULL;
        delete decodable_; decodable_ = NULL;
        delete decoder_; decoder_ = NULL;

        delete trans_model_; trans_model_ = NULL;
        delete am_; am_ = NULL;

        delete lda_mat_; lda_mat_ = NULL;
        delete cmvn_mat_; cmvn_mat_ = NULL;

        delete words_; words_ = NULL;

        delete config_; config_ = NULL;
    }

    void PyKaldi2Decoder::DestroyPipeline() {
        delete mfcc_;
        mfcc_ = NULL;
        delete cmvn_;
        cmvn_ = NULL;
        delete cmvn_state_;
        cmvn_state_ = NULL;
        delete splice_;
        splice_ = NULL;
        delete transform_lda_;
        transform_lda_ = NULL;

        delete ivector_append_;
        ivector_append_ = NULL;
        delete ivector_;
        ivector_ = NULL;
    }
} // namespace kaldi
