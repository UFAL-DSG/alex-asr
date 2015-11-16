#include "src/decoder.h"
#include "src/utils.h"

#include "online2/onlinebin-util.h"

using namespace kaldi;

namespace alex_asr {
    Decoder::Decoder(const string model_path) :
            feature_pipeline_(NULL),
            hclg_(NULL),
            decoder_(NULL),
            trans_model_(NULL),
            am_nnet2_(NULL),
            am_gmm_(NULL),
            words_(NULL),
            config_(NULL),
            decodable_(NULL)

    {
        // Change dir to model_path. Change back when leaving the scope.
        local_cwd cwd_to_model_path(model_path);
        KALDI_VLOG(2) << "Decoder is setting up models: " << model_path;

        ParseConfig();
        LoadDecoder();
        Reset();

        KALDI_VLOG(2) << "Decoder is successfully initialized.";
    }

    Decoder::~Decoder() {
        delete feature_pipeline_;
        delete hclg_;
        delete decoder_;
        delete trans_model_;
        delete am_nnet2_;
        delete am_gmm_;
        delete words_;
        delete config_;
        delete decodable_;
    }

    void Decoder::ParseConfig() {
        KALDI_PARANOID_ASSERT(config_ == NULL);

        config_ = new DecoderConfig();
        config_->LoadConfigs("pykaldi.cfg");

        if(!config_->InitAndCheck()) {
            KALDI_ERR << "Error when checking if the configuration is valid. "
                    "Please check your configuration.";
        }
    }

    void Decoder::LoadDecoder() {
        bool binary;
        Input ki(config_->model_rxfilename, &binary);

        KALDI_PARANOID_ASSERT(trans_model_ == NULL);
        trans_model_ = new TransitionModel();
        trans_model_->Read(ki.Stream(), binary);

        if(config_->model_type == DecoderConfig::GMM) {
            KALDI_PARANOID_ASSERT(am_gmm_ == NULL);
            am_gmm_ = new AmDiagGmm();
            am_gmm_->Read(ki.Stream(), binary);
        } else if(config_->model_type == DecoderConfig::NNET2) {
            KALDI_PARANOID_ASSERT(am_nnet2_ == NULL);
            am_nnet2_ = new nnet2::AmNnet();
            am_nnet2_->Read(ki.Stream(), binary);
        }

        KALDI_PARANOID_ASSERT(hclg_ == NULL);
        hclg_ = ReadDecodeGraph(config_->fst_rxfilename);
        KALDI_PARANOID_ASSERT(decoder_ == NULL);
        decoder_ = new LatticeFasterOnlineDecoder(*hclg_, config_->decoder_opts);

        KALDI_PARANOID_ASSERT(words_ == NULL);
        words_ = fst::SymbolTable::ReadText(config_->words_rxfilename);
    }

    void Decoder::Reset() {
        delete feature_pipeline_;
        delete decodable_;

        feature_pipeline_ = new FeaturePipeline(*config_);

        if(config_->model_type == DecoderConfig::GMM) {
            decodable_ = new DecodableDiagGmmScaledOnline(*am_gmm_,
                                                          *trans_model_,
                                                          config_->decodable_opts.acoustic_scale,
                                                          feature_pipeline_->GetFeature());
        } else if(config_->model_type == DecoderConfig::NNET2) {
            decodable_ = new nnet2::DecodableNnet2Online(*am_nnet2_,
                                                         *trans_model_,
                                                         config_->decodable_opts,
                                                         feature_pipeline_->GetFeature());
        } else {
            KALDI_ASSERT(false);  // This means the program is in invalid state.
        }



        decoder_->InitDecoding();
    }

    bool Decoder::EndpointDetected() {
        return kaldi::EndpointDetected(config_->endpoint_config, *trans_model_,
                                       config_->mfcc_opts.frame_opts.frame_shift_ms * 1.0e-03f,
                                       *decoder_);
    }

    void Decoder::FrameIn(VectorBase<BaseFloat> *waveform_in) {
        feature_pipeline_->AcceptWaveform(config_->mfcc_opts.frame_opts.samp_freq, *waveform_in);
    }

    void Decoder::FrameIn(unsigned char *buffer, int32 buffer_length) {
        int n_frames = buffer_length / (config_->bits_per_sample / 8);

        Vector<BaseFloat> waveform(n_frames);

        for(int32 i = 0; i < n_frames; ++i) {
            switch(config_->bits_per_sample) {
                case 8:
                {
                    waveform(i) = (*buffer);
                    buffer++;
                    break;
                }
                case 16:
                {
                    int16 k = *reinterpret_cast<uint16*>(buffer);
#ifdef __BIG_ENDDIAN__
                    KALDI_SWAP2(k);
#endif
                    waveform(i) = k;
                    buffer += 2;
                    break;
                }
                default:
                    KALDI_ERR << "Unsupported bits ber sample (implement yourself): "
                    << config_->bits_per_sample;
            }
        }
        this->FrameIn(&waveform);
    }

    void Decoder::InputFinished() {
        feature_pipeline_->InputFinished();
    }

    int32 Decoder::Decode(int32 max_frames) {
        int32 decoded = decoder_->NumFramesDecoded();
        decoder_->AdvanceDecoding(decodable_, max_frames);

        return decoder_->NumFramesDecoded() - decoded;
    }

    void Decoder::FinalizeDecoding() {
        decoder_->FinalizeDecoding();
    }

    bool Decoder::GetBestPath(std::vector<int> *out_words, BaseFloat *prob) {
        *prob = -1.0f;

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

    bool Decoder::GetLattice(fst::VectorFst<fst::LogArc> *fst_out,
                                     double *tot_lik, bool end_of_utterance) {
        CompactLattice lat;
        Lattice raw_lat;

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

    string Decoder::GetWord(int word_id) {
        return words_->Find(word_id);
    }

    float Decoder::FinalRelativeCost() {
        return decoder_->FinalRelativeCost();
    }

    int32 Decoder::NumFramesDecoded() {
        return decoder_->NumFramesDecoded();
    }

    int32 Decoder::TrailingSilenceLength() {
        if(config_->endpoint_config.silence_phones == "") {
            KALDI_WARN << "Trying to get training silence length for a model that does not have"
                          "silence phones configured.";
            return -1;
        } else {
            return kaldi::TrailingSilenceLength(*trans_model_,
                                                config_->endpoint_config.silence_phones,
                                                *decoder_);
        }
    }

    void Decoder::GetIvector(std::vector<float> *ivector) {
        if(config_->use_ivectors) {
            KALDI_WARN << "Trying to get an Ivector for a model that does not have Ivectors.";
        } else {
            OnlineIvectorFeature *ivector_ftr = feature_pipeline_->GetIvectorFeature();

            Vector<BaseFloat> ivector_res;
            ivector_res.Resize(ivector_ftr->Dim());
            ivector_ftr->GetFrame(decoder_->NumFramesDecoded() - 1, &ivector_res);

            BaseFloat *data = ivector_res.Data();
            for (int32 i = 0; i < ivector_res.Dim(); i++) {
                ivector->push_back(data[i]);
            }
        }
    }

    void Decoder::SetBitsPerSample(int n_bits) {
        KALDI_ASSERT(n_bits % 8 == 0);

        config_->bits_per_sample = n_bits;
    }

    int Decoder::GetBitsPerSample() {
        return config_->bits_per_sample;
    }
}
