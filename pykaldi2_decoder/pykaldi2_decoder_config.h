#ifndef PYKALDI_PYKALDI2_DECODER_CONFIG_H
#define PYKALDI_PYKALDI2_DECODER_CONFIG_H

#include <sys/stat.h>

#include "decoder/lattice-faster-decoder.h"
#include "decoder/lattice-faster-online-decoder.h"
#include "feat/feature-mfcc.h"
#include "feat/online-feature.h"
#include "feat/pitch-functions.h"
#include "nnet2/online-nnet2-decodable.h"
#include "online2/online-endpoint.h"
#include "online2/online-ivector-feature.h"
#include "util/stl-utils.h"
#include "pykaldi2_decoder/utils.h"



namespace kaldi {
    class PyKaldi2DecoderConfig {
    public:
        enum ModelType { None, GMM, NNET2 };

        PyKaldi2DecoderConfig();
        ~PyKaldi2DecoderConfig();
        void Register(ParseOptions *po);
        void LoadConfigs(const string cfg_file);
        bool InitAndCheck();

        LatticeFasterDecoderConfig decoder_opts;
        nnet2::DecodableNnet2OnlineOptions decodable_opts;
        MfccOptions mfcc_opts;
        OnlineCmvnOptions cmvn_opts;
        OnlineSpliceOptions splice_opts;
        OnlineEndpointConfig endpoint_config;
        OnlineIvectorExtractionConfig ivector_config;
        PitchExtractionOptions pitch_opts;
        ProcessPitchOptions pitch_process_opts;

        Matrix<BaseFloat> *lda_mat;
        Matrix<double> *cmvn_mat;
        OnlineIvectorExtractionInfo *ivector_extraction_info;

        ModelType model_type;
        int32 bits_per_sample;

        bool use_ivectors;
        bool use_cmvn;
        bool use_pitch;

        std::string cfg_decoder;
        std::string cfg_decodable;
        std::string cfg_mfcc;
        std::string cfg_cmvn;
        std::string cfg_splice;
        std::string cfg_endpoint;
        std::string cfg_ivector;
        std::string cfg_pitch;

        std::string model_rxfilename;
        std::string fst_rxfilename;
        std::string words_rxfilename;
        std::string lda_mat_rspecifier;
        std::string fcmvn_mat_rspecifier;
    private:
        void InitAux();
        void LoadLDA();
        void LoadCMVN();
        void LoadIvector();
        template<typename C> void LoadConfig(string file_name, C *opts);
        bool FileExists(string strFilename);
        bool OptionCheck(bool cond, std::string fail_text);

        string model_type_str;
    };
}

#endif //PYKALDI_PYKALDI2_DECODER_CONFIG_H
