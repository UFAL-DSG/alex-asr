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


install_requires = ['cython>=0.21', 'pystache>=0.5', 'pyyaml>=3.11']
if python_version < (2, 7):
    new_27 = ['ordereddict', 'argparse']
    install_requires.extend(new_27)

extra_link_args = []
extra_objects = environ.get('ADDLIBS', '').split()
extra_compile_args = environ.get('CXXFLAGS', '').split()
kaldi_dir = environ.get('KALDI_DIR', 'libs/kaldi')
version = environ.get('PYKALDI_VERSION', 'unknown')
library_dirs = ['/usr/lib']
libraries = ['lapack_atlas', 'cblas', 'atlas', 'f77blas', 'm', 'pthread', 'dl']


def kaldi_path(path):
    return os.path.join(kaldi_dir, path)


ext_modules = []
ext_modules.append(Extension('alex_asr.decoder',
                             language='c++',
                             extra_compile_args=extra_compile_args,
                             extra_link_args=extra_link_args,
                             include_dirs=[
                                 '.',
                                 kaldi_path('tools/openfst/include'),
                                 kaldi_path('src'),
                             ],
                             library_dirs=library_dirs,
                             libraries=libraries,
                             extra_objects=extra_objects,
                             sources=['alex_asr/decoder.pyx', ],
                             ))
ext_modules.append(Extension(name='alex_asr.fst._fst',
                                 extra_compile_args=extra_compile_args + ['-std=c++11'],
                                 extra_link_args=extra_link_args,
                                 include_dirs=[
                                     '.',
                                     kaldi_path('tools/openfst/include'),
                                     kaldi_path('src'),
                                 ],
                                 extra_objects=extra_objects,
                                 sources=['alex_asr/fst/_fst.pyx'],
                                 language='c++'
                                 )
)


setup(
    name='alex_asr',
    packages=find_packages(exclude=["alex_asr/decoder.cpp"]),
    include_package_data=True,
    cmdclass={'build_ext': build_ext},
    version=version,
    install_requires=install_requires,
    setup_requires=['cython>=0.19.1', 'nose>=1.0'],
    ext_modules=ext_modules,
    author='Lukas Zilka',
    author_email='lukas@zilka.me',
    url='https://github.com/DSG-UFAL/alex-asr',
    license='Apache, Version 2.0',
    keywords='Incremental speech recognition decoder.',
    description='',
    long_description='',
    classifiers='''
        Programming Language :: Python :: 2
        License :: OSI Approved :: Apache License, Version 2
        Operating System :: POSIX :: Linux
        Intended Audience :: Speech Recognition scientist
        Intended Audience :: Students
        Environment :: Console
        '''.strip().splitlines(),
)
