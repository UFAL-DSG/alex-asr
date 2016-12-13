# Alex-ASR

Incremental speech recognition decoder for Kaldi NNET2 and GMM models with Python bindings (tested with Python 2.7 and Python 3.4).

Python module documentation is [here](http://ufal-dsg.github.io/alex-asr/).

# Example Usage

```python
from alex_asr import Decoder
import wave
import struct
import os

# Load speech recognition model from "asr_model_dir" directory.
decoder = Decoder("asr_model_dir/")

# Load audio frames from input wav file.
data = wave.open("input.wav")
frames = data.readframes(data.getnframes())

# Feed the audio data to the decoder.
decoder.accept_audio(frames)
decoder.decode(data.getnframes())
decoder.input_finished()

# Get and print the best hypothesis.
prob, word_ids = decoder.get_best_path()
print " ".join(map(decoder.get_word, word_ids))
```

# Build & Install

## Ubuntu 14.04 requirements installation
```
apt-get update
apt-get install -y build-essential libatlas-base-dev python-dev python-pip git wget gfortran g++ unzip zlib1g-dev automake autoconf libtool subversion
pip install Cython
```

## Installation
```
$ python setup.py install
```

# Configuration

  - The decoder takes one argument `model_dir` for initialization. It is a directory with the decoder model and its configuration.
  - It expects that a file called `alex_asr.conf` is contained in it. This file specifies filenames of all other configs, and adheres to Kaldi configuration standards (i.e. one option per line in a text file).
  - All filenames specified in this config are relative to `model_dir`

Example of `alex_asr.conf` that should reside in `model_dir`:

```
--model_type=nnet2     # Supported model types are nnet2 (nnet2::AmNnet) and gmm (AmDiagGmm)
--model=final.mdl      # Filename of the mdl file for the decoder.
--hclg=HCLG.fst        # Filename of the fst file with decoding HCLG fst.
--words=words.txt      # Filename with a list of words (each line contains: ""<word> <word-id>").
--mat_lda=final.mat    # Filaneme of the LDA transform matrix.
--mat_cmvn=cmvn.mat    # Filename of the CMVN matrix with global CMVN stats used for OnlineCMVN estimator.
--use_lda=true         # true/false; Says whether to use LDA transform specified by --mat_lda on MFCC features.
--use_ivectors=true    # true/false; Says whether to use Ivector features for decoding
                       # (depends on your decoder). If set to true, you need to also specify --cfg_ivector
                       # with configuration for the ivector extractor.
--use_cmvn=false       # true/false; Says whether to do OnlineCMVN estimation. Uses --mat_cmvn as an initial
                       # matrix for OnlineCMVN estimation. If set to true, --cfg_cmvn must specify a file
                       # with configuration for the estimator.
--use_pitch=false      # true/false. Whether to use pitch feature. If true, --cfg_pitch must specify a file
                       # with configuration of the pitch extractor.
--bits_per_sample=16   # 8/16; How many bits per sample frame?

# These parameters specify filenames of configuration of the particular parts of the decoder. Detailed below.
--cfg_decoder=decoder.cfg
--cfg_decodable=decodable.cfg
--cfg_mfcc=mfcc.cfg
--cfg_cmvn=cmvn.cfg
--cfg_splice=splice.cfg
--cfg_endpoint=endpoint.cfg
--cfg_ivector=ivector.cfg
--cfg_pitch=pitch.cfg
```

## Decoder configuration.

Example ``decoder.cfg``:
```
--max-active=7000
--min-active=200
--beam=15.0
--lattice-beam=8.0
```

Details: https://github.com/kaldi-asr/kaldi/blob/master/src/decoder/lattice-faster-decoder.h#L69


## Decodable configuration.

Example ``decodable.cfg``:
```
--acoustic-scale=0.1
```

Details: https://github.com/kaldi-asr/kaldi/blob/master/src/nnet2/online-nnet2-decodable.h#L48

## MFCC configuration

Example ``mfcc.cfg``:
```
--low-freq=128
--high-freq=3800
```

Details: https://github.com/kaldi-asr/kaldi/blob/master/src/feat/feature-mfcc.h#L63

## Online CMVN configuration

Online CMVN configuration is needed when you set ``--use_cmvn=true``.

Details: https://github.com/kaldi-asr/kaldi/blob/master/src/feat/online-feature.h#L176

## Splice configuration

Example ``splice.cfg``:
```
--left-context=3
--right-context=3
```

Details: https://github.com/kaldi-asr/kaldi/blob/master/src/feat/online-feature.h#L384

## Endpoint configuration

Endpointing configuration is needed if you intend to call ``EndpointDetected`` and ``TrailingSilenceLength`` functions of the ``Decoder``.

Example ``endpoint.cfg``:

```
--endpoint.silence_phones=1:2:3:4:5:6:7:8:9:10:11:12:13:14:15:16:17:18:19:20:21:22:23:24:25
```

https://github.com/kaldi-asr/kaldi/blob/master/src/online2/online-endpoint.h#L159


## IVector configuration

Ivector configuration is needed if you set ``--use_ivectors=true``.

Example ``ivector.cfg``:

```
--splice-config=ivector_extractor/splice_opts
--cmvn-config=ivector_extractor/online_cmvn.conf
--lda-matrix=ivector_extractor/final.mat
--global-cmvn-stats=ivector_extractor/global_cmvn.stats
--diag-ubm=ivector_extractor/final.dubm
--ivector-extractor=ivector_extractor/final.ie
--num-gselect=5
--min-post=0.025
--posterior-scale=0.1
--max-remembered-frames=1000
--max-count=100
```

https://github.com/kaldi-asr/kaldi/blob/master/src/online2/online-ivector-feature.h#L110

## Pitch configuration

Pitch configuration is neede if you set ``--use_pitch=true``.

Details: https://github.com/kaldi-asr/kaldi/blob/master/src/feat/pitch-functions.h#L136

Details: https://github.com/kaldi-asr/kaldi/blob/master/src/feat/pitch-functions.h#L250

# Regenerate and publish documentation

Provided you have built the module, the documentation can be built by the following commads:
```
$ cd doc
$ bash build_and_push_to_web.sh 
```

This uses sphinx to build the documentation and pushes it as a GitHub page of the repository.

# License

Copyright 2015 Charles University in Prague

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

# Credits

Author: Lukas Zilka (lukas@zilka.me).

Adapted from Ondra Platek's PyKaldi https://github.com/UFAL-DSG/pykaldi

Integrated Cython code from: https://github.com/vchahun/pyfst
