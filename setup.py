from setuptools import setup
import os
import re
import sys

from Cython.Build import cythonize
import cffi
import numpy

# Three CFFI extension modules, one for HDF-EOS, one for HDF-EOS5, and one
# for augmenting HDF-EOS with HDF4.
import pyhdfeos
ext_modules = [pyhdfeos.lib.he4.ffi.verifier.get_extension(),
               pyhdfeos.lib.he5.ffi.verifier.get_extension(),
               pyhdfeos.lib.hdf.ffi.verifier.get_extension()]

# We need to locate libGctp (libgctp if on a debian variant) in order to 
# compile the som grid extension module.
include_dirs = pyhdfeos.lib.config.include_dirs
library_dirs = pyhdfeos.lib.config.library_dir_candidates
true_gctp_lib = pyhdfeos.lib.config.locate_gctp(library_dirs)

from distutils.extension import Extension
cythonize("pyhdfeos/_som.pyx")
e = Extension("pyhdfeos/_som", ["pyhdfeos/_som.c"],
        include_dirs = include_dirs,
        libraries    = [true_gctp_lib],
        library_dirs = library_dirs)
ext_modules.append(e)

install_requires = ['numpy>=1.8.0', 'cffi>=0.8.2', 'cython>=0.20']
if sys.hexversion < 0x03000000:
    install_requires.append('mock>=1.0.1')

classifiers = ["Programming Language :: Python",
               "Programming Language :: Python :: 2.7",
               "Programming Language :: Python :: 3.4",
               "Programming Language :: Python :: Implementation :: CPython",
               "License :: OSI Approved :: MIT License",
               "Development Status :: 3 - Production/Alpha",
               "Operating System :: MacOS",
               "Operating System :: POSIX :: Linux",
               "Intended Audience :: Science/Research",
               "Intended Audience :: Information Technology",
               "Topic :: Software Development :: Libraries :: Python Modules"]

entry_points = {'console_scripts':
        ['hedump=pyhdfeos.command_line:dump_metadata'] }

setup(name             = 'pyhdfeos',
      description      = 'Tools for accessing HDF-EOS grids',
      long_description = open('README.md').read(),
      author           = 'John Evans',
      author_email     = 'john.g.evans.ne@gmail.com',
      url              = 'http://hdfeos.org',
      packages         = ['pyhdfeos', 'pyhdfeos.lib'],
      version          = '0.1.0rc3',
      zip_safe         =  False,
      ext_package      = 'pyhdfeos',
      ext_modules      = ext_modules,
      include_dirs     = [numpy.get_include(), '/usr/include/hdf-eos5'],
      entry_points     = entry_points,
      install_requires = install_requires,
      license          = 'MIT',
      classifiers      = classifiers)
