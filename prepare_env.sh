#!/usr/bin/env bash
PYTHON=python
FSTDIR=$(python -c "import os,sys; print os.path.realpath(sys.argv[1])" libs/kaldi/tools/openfst)
OPENFST_VERSION=1.3.4
KALDI_REV=ea37842dc1f4f03819acae27a6363c993ce5d12b

mkdir libs

# Get Kaldi.
git clone https://github.com/kaldi-asr/kaldi.git libs/kaldi
(
    cd libs/kaldi/src;
    git checkout ${KALDI_REV}
)
# Get PyFST
git clone https://github.com/UFAL-DSG/pyfst.git libs/pyfst

# Prepare Kaldi dependencies.
make -C libs/kaldi/tools atlas
(
    # Patch OpenFST makefile so that we can link with it statically.
    cd libs/kaldi/tools;
    sed -i "s/--enable-ngram-fsts/--enable-ngram-fsts --with-pic/g" Makefile
    make openfst
)

# Configure Kaldi.
(
    cd libs/kaldi/src;
    git checkout ${KALDI_REV}
    ./configure --shared --use-cuda=no
)

# Build Kaldi.
make -C libs/kaldi/src
