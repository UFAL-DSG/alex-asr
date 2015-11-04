// -*- coding: utf-8 -*-
/* Copyright (c) 2013-14, Ondrej Platek, Ufal MFF UK <oplatek@ufal.mff.cuni.cz> */

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
#include <string>
#include "lat/kaldi-lattice.h"
#include "fstext/fstext-utils.h"
#include "fstext/lattice-utils-inl.h"
#include "fstext/lattice-weight.h"
#include "lat/lattice-functions.h"
#include "pykaldi2_decoder/utils.h"

namespace kaldi {

// instantiate the template used below here
// TODO
// template double ComputeLatticeAlphasAndBetas<CompactLattice>;

  void MovePostToArcs(const std::vector<double> &alpha,
                      const std::vector<double> &beta,
                      fst::VectorFst<fst::LogArc> * lat) {
    using namespace fst;
    typedef LogArc::StateId StateId;
    StateId num_states = lat->NumStates();
    for (StateId i = 0; i < num_states; ++i) {
      for (MutableArcIterator<VectorFst<LogArc> > aiter(lat, i);
          !aiter.Done();
           aiter.Next()) {
        LogArc arc = aiter.Value();
        StateId j = arc.nextstate;
        // w(i,j) = alpha(i) * w(i,j) * beta(j) / (alpha(i) * beta(i))
        // w(i,j) = w(i,j) * beta(j) / beta(i)
        double orig_w = ConvertToCost(arc.weight);
        double numer = orig_w + -beta[j];
        KALDI_VLOG(3) << "arc(" << i << ',' << j << ')' << std::endl <<
          "orig_w:" << orig_w << " beta[j=" << j << "]:" << -beta[j] <<
          " beta[i=" << i << "]:" << -beta[i] << " numer:" << numer << std::endl;
        double new_w = numer - (-beta[i]);
        KALDI_VLOG(3) << "arc orig: " << orig_w << " new: " << new_w << std::endl;
        arc.weight = LogWeight(new_w);

        aiter.SetValue(arc);
      }
    }
  }

  double CompactLatticeToWordsPost(CompactLattice &clat, fst::VectorFst<fst::LogArc> *pst) {

    {
      Lattice lat;
      fst::VectorFst<fst::StdArc> t_std;
      RemoveAlignmentsFromCompactLattice(&clat); // remove the alignments
      ConvertLattice(clat, &lat); // convert to non-compact form.. no new states
      ConvertLattice(lat, &t_std); // this adds up the (lm,acoustic) costs
      fst::Cast(t_std, pst);  // reinterpret the inner implementations
    }
    fst::Project(pst, fst::PROJECT_OUTPUT);


    fst::Minimize(pst);

    fst::ArcMap(pst, fst::SuperFinalMapper<fst::LogArc>());

    double tot_lik;
    std::vector<double> alpha, beta;
    fst::TopSort(pst);
    // tot_lik = ComputeCompactLatticeAlphasAndBetas(*pst, &alpha, &beta); // TODO
    tot_lik = ComputeLatticeAlphasAndBetas(*pst, &alpha, &beta);
    MovePostToArcs(alpha, beta, pst);

    return tot_lik;
  }

  template<typename LatticeType>
  static double ComputeLatticeAlphasAndBetas(const LatticeType &lat,
                                             vector<double> *alpha,
                                             vector<double> *beta) {
    typedef typename LatticeType::Arc Arc;
    typedef typename Arc::Weight Weight;
    typedef typename Arc::StateId StateId;

    StateId num_states = lat.NumStates();
    KALDI_ASSERT(lat.Properties(fst::kTopSorted, true) == fst::kTopSorted);
    KALDI_ASSERT(lat.Start() == 0);
    alpha->resize(num_states, kLogZeroDouble);
    beta->resize(num_states, kLogZeroDouble);

    double tot_forward_prob = kLogZeroDouble;
    (*alpha)[0] = 0.0;
    // Propagate alphas forward.
    for (StateId s = 0; s < num_states; s++) {
      double this_alpha = (*alpha)[s];
      for (fst::ArcIterator<LatticeType> aiter(lat, s); !aiter.Done();
           aiter.Next()) {
        const Arc &arc = aiter.Value();
        double arc_like = -ConvertToCost(arc.weight);
        (*alpha)[arc.nextstate] = LogAdd((*alpha)[arc.nextstate],
                                         this_alpha + arc_like);
      }
      Weight f = lat.Final(s);
      if (f != Weight::Zero()) {
        double final_like = this_alpha - ConvertToCost(f);
        tot_forward_prob = LogAdd(tot_forward_prob, final_like);
      }
    }
    for (StateId s = num_states-1; s >= 0; s--) { // it's guaranteed signed.
      double this_beta = -ConvertToCost(lat.Final(s));
      for (fst::ArcIterator<LatticeType> aiter(lat, s); !aiter.Done();
           aiter.Next()) {
        const Arc &arc = aiter.Value();
        double arc_like = -ConvertToCost(arc.weight),
                arc_beta = (*beta)[arc.nextstate] + arc_like;
        this_beta = LogAdd(this_beta, arc_beta);
      }
      (*beta)[s] = this_beta;
    }
    double tot_backward_prob = (*beta)[lat.Start()];
    if (!ApproxEqual(tot_forward_prob, tot_backward_prob, 1e-8)) {
      KALDI_WARN << "Total forward probability over lattice = " << tot_forward_prob
      << ", while total backward probability = " << tot_backward_prob;
    }
    // Split the difference when returning... they should be the same.
    return 0.5 * (tot_backward_prob + tot_forward_prob);
  }

    const string GetDirectory(const string& file_name) {
        size_t found;
        found = file_name.find_last_of("/\\");
        return file_name.substr(0,found);
    }

} // namespace kaldi
