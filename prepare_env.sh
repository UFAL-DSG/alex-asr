#!/usr/bin/env bash
PYTHON=python
FSTDIR=$(realpath libs/kaldi/tools/openfst)
OPENFST_VERSION=1.3.4

mkdir libs

git clone https://github.com/UFAL-DSG/kaldi-git.git libs/kaldi
git clone https://github.com/UFAL-DSG/pyfst.git libs/pyfst

make -C libs/kaldi/tools  atlas ; echo "Installing atlas finished $?"
make -C libs/kaldi/tools openfst OPENFST_VERSION=${OPENFST_VERSION}; echo "Installing OpenFST finished: $?"

(cd libs/kaldi/src; ./configure --shared)

make -C libs/kaldi/src


pushd libs/pyfst
LIBRARY_PATH=${FSTDIR}/lib:${FSTDIR}/lib/fst CPLUS_INCLUDE_PATH=${FSTDIR}/include ${PYTHON} setup.py build_ext --inplace
popd

