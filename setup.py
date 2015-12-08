#!/usr/bin/env python
# encoding: utf-8
# cython: embedsignature=True

from __future__ import print_function
from setuptools import setup, find_packages
from sys import version_info as python_version
import os
from distutils.extension import Extension
from Cython.Distutils import build_ext
from os import environ
from sys import stderr
from sys import platform

curr_dir = os.path.dirname(__file__)


install_requires = ['cython>=0.21', 'pystache>=0.5', 'pyyaml>=3.11']
if python_version < (2, 7):
    new_27 = ['ordereddict', 'argparse']
    install_requires.extend(new_27)

kaldi_dir = environ.get('KALDI_DIR', 'libs/kaldi')
library_dirs = ['/usr/lib']
libraries = ['lapack_atlas', 'cblas', 'atlas', 'f77blas', 'm', 'pthread', 'dl']


def kaldi_path(path):
    return os.path.join(kaldi_dir, path)


class build_ext_with_make(build_ext):
    def run(self):
        os.system('(cd "%s"; bash prepare_env.sh; make)' % curr_dir)

        with open('setup.py.add_libs') as f_in:
            libs = f_in.read().split()
            for ext in self.extensions:
                ext.extra_objects.extend(libs)

        with open('setup.py.cxxflags_kaldi') as f_in:
            flags = f_in.read().split()
            for ext in self.extensions:
                if ext.name =='alex_asr.decoder':
                    ext.extra_compile_args.extend(flags)

        with open('setup.py.cxxflags_pyfst') as f_in:
            flags = f_in.read().split()
            for ext in self.extensions:
                if ext.name =='alex_asr.fst._fst':
                    ext.extra_compile_args.extend(flags)

        build_ext.run(self)


setup(
    name='alex_asr',
    packages=find_packages(exclude=["alex_asr/decoder.cpp"]),
    include_package_data=True,
    cmdclass={'build_ext': build_ext_with_make},
    version='1.0.4',
    install_requires=install_requires,
    setup_requires=['cython>=0.19.1', 'nose>=1.0'],
    ext_modules=[
        Extension('alex_asr.decoder',
            language='c++',
            include_dirs=[
                '.',
                kaldi_path('tools/openfst/include'),
                kaldi_path('src'),
            ],
            library_dirs=library_dirs,
            libraries=libraries,
            sources=['alex_asr/decoder.pyx', ],
        ),
        Extension(name='alex_asr.fst._fst',
            extra_compile_args=['-std=c++0x'],  # NOTE: This is for backward compatibility with gcc 4.6 which is in ubuntu 12.04. Ideally we should use '-std=c++11'.
            include_dirs=[
                '.',
                kaldi_path('tools/openfst/include'),
                kaldi_path('src'),
            ],
            sources=['alex_asr/fst/_fst.pyx'],
            language='c++'
        )
    ],
    author='Charles University in Prague (Lukas Zilka)',
    author_email='lukas@zilka.me',
    url='https://github.com/UFAL-DSG/alex-asr',
    license='Apache Software License 2.0',
    keywords='asr, speech recognition, decoder, speech',
    description='Incremental speech recognition decoder for Kaldi NNET2 and GMM models.',
    download_url='https://github.com/UFAL-DSG/alex-asr/archive/v1.0.4.tar.gz',
    long_description='',
    classifiers='''
        Programming Language :: Python :: 2
        Programming Language :: Python :: 3
        License :: OSI Approved :: Apache Software License
        Operating System :: POSIX :: Linux
        Topic :: Multimedia :: Sound/Audio :: Speech
        Topic :: Scientific/Engineering
        Intended Audience :: Developers
        Intended Audience :: Science/Research
        Intended Audience :: Telecommunications Industry
        Environment :: Console
        '''.strip().splitlines(),
)
