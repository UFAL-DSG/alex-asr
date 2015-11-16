// Copyright 2012 Cisco Systems (author: Matthias Paulik)

//   Modifications to the original contribution by Cisco Systems made by:
//   Vassil Panayotov
//   Ondrej Platek

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.
#ifndef PYKALDI2_UTILS_H_
#define PYKALDI2_UTILS_H_
#include <string>
#include <sys/param.h>
#include "base/kaldi-common.h"
#include "fstext/fstext-lib.h"
#include "lat/kaldi-lattice.h"


#ifdef DEBUG
#include <fstream>
#endif // DEBUG

using namespace kaldi;

namespace fst {
    template<class Float>
    inline double ConvertToCost(const LogWeightTpl<Float> &w) {
        return w.Value();
    }
}

namespace alex_asr {

    /// \addtogroup online_latgen_utils
    /// @{

    // typedef double ComputeLatticeAlphasAndBetas<CompactLattice> ComputeCompactLatticeAlphasAndBetas;


    // Lattice lat has to have loglikelihoods on weights
    void MovePostToArcs(const std::vector<double> &alpha,
                        const std::vector<double> &beta,
                        fst::VectorFst<fst::LogArc> * lat);


    // the input lattice has to have log-likelihood weights
    double CompactLatticeToWordsPost(CompactLattice &lat, fst::VectorFst<fst::LogArc> *pst);

    /// @} end of "addtogroup online_latgen_utils"

    template<typename LatticeType>
    static double ComputeLatticeAlphasAndBetas(const LatticeType &lat,
                                               vector<double> *alpha,
                                               vector<double> *beta);

    const string GetDirectory(const string& file_name);

    class local_cwd
    {
        string orig_dir_;

    public:
        local_cwd(string dir)
        {
            char temp[MAXPATHLEN];
            orig_dir_ = ( getcwd(temp, MAXPATHLEN) ? std::string( temp ) : std::string("") );

            chdir(dir.c_str());
        }
        ~local_cwd()
        {
            chdir(orig_dir_.c_str());
        }
    };
} // namespace kaldi

#endif // KALDI_DEC_WRAP_UTILS_H_
