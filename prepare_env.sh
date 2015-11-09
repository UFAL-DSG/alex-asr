#!/usr/bin/env bash
PYTHON=python
FSTDIR=$(python -c "import os,sys; print os.path.realpath(sys.argv[1])" libs/kaldi/tools/openfst)
OPENFST_VERSION=1.3.4

mkdir libs

git clone https://github.com/UFAL-DSG/kaldi-git.git libs/kaldi
git clone https://github.com/UFAL-DSG/pyfst.git libs/pyfst

make -C libs/kaldi/tools  atlas ; echo "Installing atlas finished $?"
make -C libs/kaldi/tools openfst OPENFST_VERSION=${OPENFST_VERSION}; echo "Installing OpenFST finished: $?"

(cd libs/kaldi/src; ./configure --shared)

cp libs/kaldi/tools/openfst/lib/libfst.so /usr/lib/

make -C libs/kaldi/src

pip install pyyaml
pushd libs/pyfst
LIBRARY_PATH=${FSTDIR}/lib:${FSTDIR}/lib/fst CPLUS_INCLUDE_PATH=${FSTDIR}/include ${PYTHON} setup.py build_ext --inplace
LIBRARY_PATH=${FSTDIR}/lib:${FSTDIR}/lib/fst CPLUS_INCLUDE_PATH=${FSTDIR}/include ${PYTHON} setup.py install
popd

ldconfig