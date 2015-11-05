#include "pykaldi2_decoder.h"
#include "pykaldi2_decoder/utils.h"

#include "online2/onlinebin-util.h"


namespace kaldi {
    PyKaldi2Decoder::PyKaldi2Decoder(const string model_path) :
            feature_pipeline_(NULL),
            hclg_(NULL),
            decoder_(NULL),
            trans_model_(NULL),
            am_(NULL),
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

    PyKaldi2Decoder::~PyKaldi2Decoder() {
        delete feature_pipeline_;
        delete hclg_;
        delete decoder_;
        delete trans_model_;
        delete am_;
        delete words_;
        delete config_;
        delete decodable_;
    }

    void PyKaldi2Decoder::ParseConfig() {
        KALDI_PARANOID_ASSERT(config_ == NULL);

        config_ = new PyKaldi2DecoderConfig();
        config_->LoadConfigs("pykaldi.cfg");

        if(!config_->Check()) {
            KALDI_ERR << "Error when checking if the configuration is valid. "
                    "Please check your configuration.";
        }
    }

    void PyKaldi2Decoder::LoadDecoder() {
        bool binary;
        Input ki(config_->model_rxfilename, &binary);

        KALDI_PARANOID_ASSERT(trans_model_ == NULL);
        trans_model_ = new TransitionModel();
        trans_model_->Read(ki.Stream(), binary);

        KALDI_PARANOID_ASSERT(am_ == NULL);
        am_ = new nnet2::AmNnet();
        am_->Read(ki.Stream(), binary);

        KALDI_PARANOID_ASSERT(hclg_ == NULL);
        hclg_ = ReadDecodeGraph(config_->fst_rxfilename);
        KALDI_PARANOID_ASSERT(decoder_ == NULL);
        decoder_ = new LatticeFasterOnlineDecoder(*hclg_, config_->decoder_opts);

        KALDI_PARANOID_ASSERT(words_ == NULL);
        words_ = fst::SymbolTable::ReadText(config_->words_rxfilename);
    }

    void PyKaldi2Decoder::Reset() {
        delete feature_pipeline_;
        delete decodable_;

        feature_pipeline_ = new PyKaldi2FeaturePipeline(*config_);
        decodable_ = new nnet2::DecodableNnet2Online(*am_,
                                                     *trans_model_,
                                                     config_->decodable_opts,
                                                     feature_pipeline_->GetFeature());

        decoder_->InitDecoding();
    }

    bool PyKaldi2Decoder::EndpointDetected() {
        return kaldi::EndpointDetected(config_->endpoint_config, *trans_model_,
                                       config_->mfcc_opts.frame_opts.frame_shift_ms * 1.0e-03f,
                                       *decoder_);
    }

    void PyKaldi2Decoder::FrameIn(VectorBase<BaseFloat> *waveform_in) {
        feature_pipeline_->AcceptWaveform(config_->mfcc_opts.frame_opts.samp_freq, *waveform_in);
    }

    void PyKaldi2Decoder::FrameIn(unsigned char *frame, int32 frame_len) {
        Vector<BaseFloat> waveform(frame_len);

        for(int32 i = 0; i < frame_len; ++i) {
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
        feature_pipeline_->InputFinished();
    }

    int32 PyKaldi2Decoder::Decode(int32 max_frames) {
        int32 decoded = decoder_->NumFramesDecoded();
        decoder_->AdvanceDecoding(decodable_, max_frames);

        return decoder_->NumFramesDecoded() - decoded;
    }

    void PyKaldi2Decoder::FinalizeDecoding() {
        decoder_->FinalizeDecoding();
    }

    bool PyKaldi2Decoder::GetBestPath(std::vector<int> *out_words, BaseFloat *prob) {
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

    bool PyKaldi2Decoder::GetLattice(fst::VectorFst<fst::LogArc> *fst_out,
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

    string PyKaldi2Decoder::GetWord(int word_id) {
        return words_->Find(word_id);
    }
} // namespace kaldi
