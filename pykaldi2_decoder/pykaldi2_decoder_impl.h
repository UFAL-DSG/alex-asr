//
// Created by zilka on 11/5/15.
//

#ifndef PYKALDI_PYKALDI2_DECODER_IMPL_H
#define PYKALDI_PYKALDI2_DECODER_IMPL_H

#include "pykaldi2_decoder/pykaldi2_decoder_config.h"


namespace kaldi {

/// \addtogroup online_latgen
/// @{

    class PyKaldi2DecoderImpl {
    public:
        PyKaldi2DecoderImpl::PyKaldi2DecoderImpl(PyKaldi2DecoderConfig &config);

    private:
        std::shared_ptr<fst::StdFst> hclg_;

        std::shared_ptr<LatticeFasterOnlineDecoder> decoder_;

        std::shared_ptr<TransitionModel> trans_model_;
        std::shared_ptr<nnet2::AmNnet> am_;

        std::shared_ptr<fst::SymbolTable> words_;

    };
}

#endif //PYKALDI_PYKALDI2_DECODER_IMPL_H
