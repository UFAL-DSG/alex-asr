LIBNAME = alex_asr

KALDI_DIR = libs/kaldi
PYFST_DIR = libs/pyfst

CC = g++
CXX = g++
AR = ar
AS = as
RANLIB = ranlib

FSTROOT = $(KALDI_DIR)/tools/openfst/
LIBFILE = $(LIBNAME).a

OBJFILES = src/decoder.o src/utils.o src/feature_pipeline.o \
           src/decoder_config.o src/decoder_cli.o
BINFILES = src/decoder_cli

CXXFLAGS = -msse -msse2 -Wall \
	   -pthread \
      -DKALDI_DOUBLEPRECISION=0 -DHAVE_POSIX_MEMALIGN \
      -Wno-sign-compare -Wno-unused-local-typedefs -Winit-self \
      -DHAVE_EXECINFO_H=1 -rdynamic -DHAVE_CXXABI_H \
      -DHAVE_ATLAS \
      -g \
      -I$(FSTROOT)/include \
      -I$(KALDI_DIR)/src \
      -I$(KALDI_DIR)/tools/ATLAS/include \
      -I$(KALDI_DIR)/tools/CLAPACK \
      -Wno-sign-compare -I. -fPIC

ADDLIBS = $(FSTROOT)/src/lib/.libs/libfst.a \
          $(KALDI_DIR)/src/online2/kaldi-online2.a \
          $(KALDI_DIR)/src/ivector/kaldi-ivector.a \
          $(KALDI_DIR)/src/nnet2/kaldi-nnet2.a \
          $(KALDI_DIR)/src/lat/kaldi-lat.a \
          $(KALDI_DIR)/src/decoder/kaldi-decoder.a  \
          $(KALDI_DIR)/src/cudamatrix/kaldi-cudamatrix.a \
          $(KALDI_DIR)/src/feat/kaldi-feat.a \
          $(KALDI_DIR)/src/transform/kaldi-transform.a \
          $(KALDI_DIR)/src/gmm/kaldi-gmm.a \
          $(KALDI_DIR)/src/thread/kaldi-thread.a \
          $(KALDI_DIR)/src/hmm/kaldi-hmm.a \
          $(KALDI_DIR)/src/tree/kaldi-tree.a \
          $(KALDI_DIR)/src/matrix/kaldi-matrix.a \
          $(KALDI_DIR)/src/util/kaldi-util.a \
          $(KALDI_DIR)/src/base/kaldi-base.a

LDFLAGS = $(ADDLIBS) -llapack_atlas -lcblas -latlas -lf77blas -lm -lpthread -ldl

all: $(LIBFILE) $(BINFILES) py

$(LIBFILE): $(OBJFILES)
	$(AR) -cru $(LIBNAME).a $(OBJFILES)
	$(RANLIB) $(LIBNAME).a

$(BINFILES): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

py: $(LIBFILE)
	CXXFLAGS="$(CXXFLAGS)" \
	ADDLIBS="$(LIBNAME).a $(ADDLIBS)" \
	LIBRARY_PATH=$(FSTROOT)/lib:$(FSTROOT)/lib/fst \
	    python setup.py build_ext build

clean:
	rm -rf build
	rm -f $(LIBFILE)
	rm -f $(OBJFILES) $(BINFILES)

test:
	(PYTHONPATH=$(shell echo build/lib.*) python test/test.py )


