# PyKaldi2

Online decoder for Kaldi NNET2 and GMM models with Python bindings.

# Build & Install

```
$ prepare_env.sh
$ make py
$ python setup.py install
```

# Config

  - The decoder takes one argument `model_dir` for initialization. It is a directory with the decoder model and its configuration.
  - It expects that a file called `pykaldi.cfg` is contained in it. This file specifies filenames of all other configs, and adheres to Kaldi configuration standards (i.e. one option per line in a text file).
  - All filenames specified in this config are relative to `model_dir`

Example of `pykaldi.cfg` that should reside in `model_dir`:

```
--model_type=nnet2     # Supported model types are nnet2 (nnet2::AmNnet) and gmm (AmDiagGmm)
--model=final.mdl      # Filename of the mdl file for the decoder.
--hclg=HCLG.fst        # Filename of the fst file with decoding HCLG fst.
--words=words.txt      # Filename with a list of words (each line contains: ""<word> <word-id>").
--mat_lda=final.mat    # Filaneme of the LDA transform matrix.
--mat_cmvn=cmvn.mat    # Filename of the CMVN matrix with global CMVN stats used for OnlineCMVN estimator.
--use_ivectors=true    # true/false; Says whether to use Ivector features for decoding
                       # (depends on your decoder). If set to true, you need to also specify --cfg_ivector
                       # with configuration for the ivector extractor.
--use_cmvn=false       # true/false; Says whether to do OnlineCMVN estimation. Uses --mat_cmvn as an initial
                       # matrix for OnlineCMVN estimation. If set to true, --cfg_cmvn must specify a file
                       # with configuration for the estimator.
--use_pitch=false      # true/false. Whether to use pitch feature. If true, --cfg_pitch must specify a file
                       # with configuration of the pitch extractor.
--bits_per_sample=16   # 8/16; How many bits per sample frame?

# These parameters specify filenames of configuration of the particular parts of the decoder.
--cfg_decoder=cfg.decoder
--cfg_decodable=cfg.decodable
--cfg_mfcc=cfg.mfcc
--cfg_cmvn=cfg.cmvn
--cfg_splice=cfg.splice
--cfg_endpoint=cfg.endpoint
--cfg_ivector=cfg.ivector
--cfg_pitch=cfg.pitch
```

# Credits

Author: Lukas Zilka (lukas@zilka.me). Adapted from Ondra Platek's PyKaldi https://github.com/UFAL-DSG/pykaldi
