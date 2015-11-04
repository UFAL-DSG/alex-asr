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
    struct PyKaldi2DecoderConfig {
        LatticeFasterDecoderConfig decoder_opts;
        nnet2::DecodableNnet2OnlineOptions decodable_opts;
        MfccOptions mfcc_opts;
        OnlineCmvnOptions cmvn_opts;
        OnlineSpliceOptions splice_opts;
        OnlineEndpointConfig endpoint_config;
        OnlineIvectorExtractionConfig ivector_config;
        PitchExtractionOptions pitch_opts;
        ProcessPitchOptions pitch_process_opts;

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

        PyKaldi2DecoderConfig() :
                bits_per_sample(16),
                use_ivectors(false),
                use_cmvn(false),
                use_pitch(false),
                cfg_decoder("cfg.decoder"),
                cfg_decodable("cfg.decodable"),
                cfg_mfcc("cfg.mfcc"),
                cfg_cmvn("cfg.cmvn"),
                cfg_splice("cfg.splice"),
                cfg_endpoint("cfg.endpoint"),
                cfg_ivector("cfg.ivector"),
                cfg_pitch("cfg.pitch")

        {
            decodable_opts.acoustic_scale = 0.1;
            splice_opts.left_context = 3;
            splice_opts.right_context = 3;
        }

        void Register(ParseOptions *po) {
            po->Register("model", &model_rxfilename, "Accoustic model filename.");
            po->Register("hclg", &fst_rxfilename, "HCLG FST filename.");
            po->Register("words", &words_rxfilename, "Word to ID mapping filename.");
            po->Register("mat_lda", &lda_mat_rspecifier, "LDA matrix filename.");
            po->Register("mat_cmvn", &fcmvn_mat_rspecifier, "CMVN matrix filename.");
            po->Register("use_ivectors", &use_ivectors, "Are we using ivector features?");
            po->Register("use_cmvn", &use_cmvn, "Are we using cmvn transform?");
            po->Register("use_pitch", &use_pitch, "Are we using pitch feature?");
            po->Register("bits_per_sample", &bits_per_sample, "Bits per sample for input.");

            po->Register("cfg_decoder", &cfg_decoder, "");
            po->Register("cfg_decodable", &cfg_decodable, "");
            po->Register("cfg_mfcc", &cfg_mfcc, "");
            po->Register("cfg_cmvn", &cfg_cmvn, "");
            po->Register("cfg_splice", &cfg_splice, "");
            po->Register("cfg_endpoint", &cfg_endpoint, "");
            po->Register("cfg_ivector", &cfg_ivector, "");
            po->Register("cfg_pitch", &cfg_pitch, "");
        }

        void LoadConfigs(const string cfg_file) {
            std::string model_path("");

            ParseOptions po("");
            Register(&po);

            KALDI_LOG << "Reading config file.";
            po.ReadConfigFile(cfg_file);

            KALDI_LOG << "Reading other configs.";
            LoadConfig(model_path, cfg_decoder, &decoder_opts);
            LoadConfig(model_path, cfg_decodable, &decodable_opts);
            LoadConfig(model_path, cfg_mfcc, &mfcc_opts);
            LoadConfig(model_path, cfg_cmvn, &cmvn_opts);
            LoadConfig(model_path, cfg_splice, &splice_opts);
            LoadConfig(model_path, cfg_endpoint, &endpoint_config);
            LoadConfig(model_path, cfg_ivector, &ivector_config);
            LoadConfig(model_path, cfg_pitch, &pitch_opts);
            LoadConfig(model_path, cfg_pitch, &pitch_process_opts);
        }

        template<typename C>
        void LoadConfig(string model_path, string cfg, C *opts) {
            string file_name(GetFileName(model_path, cfg));

            KALDI_LOG<< "Loading config: " << file_name;

            if(FileExists(file_name)) {
                ReadConfigFromFile(file_name, opts);
                std::cerr << "OK" << std::endl;
            } else {
                std::cerr << "Does not exist." << std::endl;
            }
        }

        const string GetFileName(string path, string file_name) {
            return file_name;

            if(file_name.length() == 0) {
                return file_name;
            } else {
                if (file_name[0] == '/')
                    return file_name;
                else
                    return path + "/" + file_name;
            }
        }

        bool FileExists(string strFilename) {
            struct stat stFileInfo;
            bool blnReturn;
            int intStat;

            // Attempt to get the file attributes
            intStat = stat(strFilename.c_str(), &stFileInfo);
            if (intStat == 0) {
                // We were able to get the file attributes
                // so the file obviously exists.
                blnReturn = true;
            } else {
                // We were not able to get the file attributes.
                // This may mean that we don't have permission to
                // access the folder which contains this file. If you
                // need to do that level of checking, lookup the
                // return values of stat which will give you
                // more details on why stat failed.
                blnReturn = false;
            }

            return blnReturn;
        }

        bool Check() {
            bool res = true;
            res |= OptionCheck(use_ivectors && cfg_ivector == "",
                               "You have to specify --cfg_ivector if you want to use ivectors.");
            res |= OptionCheck(use_cmvn && fcmvn_mat_rspecifier == "",
                               "You have to specify --cfg_cmvn if you want to use CMVN.");
            res |= OptionCheck(use_pitch && cfg_pitch == "",
                               "You have to specify --cfg_pitch if you want to use pitch.");

            res |= OptionCheck(model_rxfilename == "",
                               "You have to specify --model.");

            res |= OptionCheck(fst_rxfilename == "",
                               "You have to specify --hclg.");

            res |= OptionCheck(words_rxfilename == "",
                               "You have to specify --words.");

            res |= OptionCheck(lda_mat_rspecifier == "",
                               "You have to specify --mat_lda.");

            return res;
        }

        bool OptionCheck(bool cond, std::string fail_text) {
            if(cond) {
                KALDI_ERR << fail_text;
                return false;
            }
            return true;
        }
    };
}

#endif //PYKALDI_PYKALDI2_DECODER_CONFIG_H
