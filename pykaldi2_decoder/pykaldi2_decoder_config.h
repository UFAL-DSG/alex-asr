#ifndef PYKALDI_PYKALDI2_DECODER_CONFIG_H
#define PYKALDI_PYKALDI2_DECODER_CONFIG_H

#include "decoder/lattice-faster-decoder.h"
#include "decoder/lattice-faster-online-decoder.h"
#include "nnet2/online-nnet2-decodable.h"
#include "util/stl-utils.h"
#include "feat/feature-mfcc.h"
#include "feat/online-feature.h"
#include "online2/online-endpoint.h"


namespace kaldi {
    struct PyKaldi2DecoderConfig {
        LatticeFasterDecoderConfig decoder_opts;
        nnet2::DecodableNnet2OnlineOptions decodable_opts;
        MfccOptions mfcc_opts;
        OnlineCmvnOptions cmvn_opts;
        OnlineSpliceOptions splice_opts;
        OnlineEndpointConfig endpoint_config;

        int32 bits_per_sample;

        std::string model_rxfilename;
        std::string fst_rxfilename;
        std::string words_rxfilename;
        std::string lda_mat_rspecifier;
        std::string fmllr_mat_rspecifier;
        std::string fcmvn_mat_rspecifier;

        PyKaldi2DecoderConfig() : bits_per_sample(16) {
            decodable_opts.acoustic_scale = 0.1;
            splice_opts.left_context = 3;
            splice_opts.right_context = 3;
        }

        void Register(ParseOptions *po) {
            po->Register("model", &model_rxfilename, "Accoustic model filename.");
            po->Register("hclg", &fst_rxfilename, "HCLG FST filename.");
            po->Register("words", &words_rxfilename, "Word to ID mapping filename.");
            po->Register("mat_lda", &lda_mat_rspecifier, "LDA matrix filename.");
            po->Register("mat_fmllr", &fmllr_mat_rspecifier, "fMLLR matrix filename.");
            po->Register("mat_cmvn", &fcmvn_mat_rspecifier, "CMVN matrix filename.");

            decoder_opts.Register(po);
            decodable_opts.Register(po);
            mfcc_opts.Register(po);
            cmvn_opts.Register(po);
            splice_opts.Register(po);
            endpoint_config.Register(po);
        }

        bool Check() {
            return model_rxfilename != "" &&
                   fst_rxfilename != "" &&
                   words_rxfilename != "" &&
                   lda_mat_rspecifier != "" &&
                   fmllr_mat_rspecifier != "" &&
                   fcmvn_mat_rspecifier != "";
        }
    };
}

#endif //PYKALDI_PYKALDI2_DECODER_CONFIG_H
