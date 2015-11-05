#!/usr/bin/env python
# encoding: utf-8
# On Windows, you need to execute:
# set VS90COMNTOOLS=%VS100COMNTOOLS%
# python setup.py build_ext --compiler=msvc
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


ext_modules = []
# pykaldi static library compilation (extension is always built as shared)
try:
    extra_objects = environ['PYKALDI_ADDLIBS'].split()
except:
    print('Specify pykaldi dependant libraries in PYKALDI_ADDLIBS shell variable', file=stderr)
    extra_objects = []

try:
    version = environ['PYKALDI_VERSION']
except:
    version = 'dev-unknown'

extra_compile_args = []  #'-std=c++11', '-DNO_KALDI_HEADERS']

extra_compile_args.extend(environ.get('CXXFLAGS', '').split())

try:
    pyfst_dir = environ['PYFST_DIR']
    kaldi_dir = environ['KALDI_DIR']
except:
    pyfst_dir = 'libs/pyfst'
    kaldi_dir = 'libs/kaldi'

def kaldi_path(path):
    return os.path.join(kaldi_dir, path)

extra_link_args = []

#TODO compilation flags are prepared only for ubuntu 14.04 and OSX 10.10 64bit version
if platform == 'darwin':
    extra_compile_args.append('-stdlib=libstdc++')
    extra_compile_args.extend(['-arch i386', '-arch x86_64'])
    extra_link_args.append('-stdlib=libstdc++')
    library_dirs = []
    libraries = [kaldi_path('/tools/openfst/lib/libfst.a'), 'dl', 'm', 'pthread', ]
else:
    library_dirs = ['/usr/lib', kaldi_path('tools/openfst/lib')]
    libraries = ['fst', 'lapack_atlas', 'cblas', 'atlas', 'f77blas', 'm', 'pthread', 'dl']
ext_modules.append(Extension('kaldi2.decoders',
                             language='c++',
                             extra_compile_args=extra_compile_args,
                             extra_link_args=extra_link_args,
                             include_dirs=[
                                 '.',
                                 kaldi_path('tools/openfst/include'),
                                 kaldi_path('src'),
                                 pyfst_dir,
                             ],
                             library_dirs=library_dirs,
                             libraries=libraries,
                             extra_objects=extra_objects,
                             sources=['kaldi2/decoders.pyx', ],
                             ))


setup(
    name='kaldi2',
    packages=find_packages(exclude=["kaldi2/decoders.cpp"]),
    include_package_data=True,
    cmdclass={'build_ext': build_ext},
    version=version,
    install_requires=install_requires,
    setup_requires=['cython>=0.19.1', 'nose>=1.0'],
    ext_modules=ext_modules,
    test_suite="nose.collector",
    tests_require=['nose>=1.0', 'kaldi2'],
    author='Lukas Zilka',
    author_email='lukas@zilka.me',
    url='https://github.com/DSG-UFAL/pykaldi2',
    license='Apache, Version 2.0',
    keywords='Kaldi speech recognition Python bindings',
    description='C++/Python wrapper for Kaldi decoders',
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