import os
from setuptools import setup
import sys

from Cython.Build import cythonize
import numpy

import pyhdfeos

# Three CFFI extension modules, one for HDF-EOS, one for HDF-EOS5, and one
# for augmenting HDF-EOS with HDF4.
ext_modules = [pyhdfeos.lib.core.ffi.verifier.get_extension(),
               pyhdfeos.lib.he2.ffi.verifier.get_extension(),
               pyhdfeos.lib.he5.ffi.verifier.get_extension(),
               pyhdfeos.lib.hdf.ffi.verifier.get_extension()]

from distutils.extension import Extension
cythonize("pyhdfeos/_som.pyx")
sources = [os.path.join('pyhdfeos', '_som.c'),
           os.path.join('pyhdfeos', 'lib', 'source', 'gctp', 'cproj.c'),
           os.path.join('pyhdfeos', 'lib', 'source', 'gctp', 'report.c'),
           os.path.join('pyhdfeos', 'lib', 'source', 'gctp', 'paksz.c'),
           os.path.join('pyhdfeos', 'lib', 'source', 'gctp', 'sphdz.c'),
           os.path.join('pyhdfeos', 'lib', 'source', 'gctp', 'sominv.c'),
           os.path.join('pyhdfeos', 'lib', 'source', 'gctp_som', 'inv_init.c')]
include_dirs = ["pyhdfeos/lib/source/gctp_som",
                "pyhdfeos/lib/source/gctp"]
e = Extension("pyhdfeos/_som", sources=sources, include_dirs=include_dirs)
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
                ['hedump=pyhdfeos.command_line:dump_metadata']}

kwargs = {'name':             'pyhdfeos',
          'description':      'Tools for accessing HDF-EOS grids',
          'long_description': open('README.md').read(),
          'author':           'John Evans, Joe Lee',
          'author_email':     'john.g.evans.ne@gmail.com',
          'url':              'http://hdfeos.org',
          'packages':         ['pyhdfeos', 'pyhdfeos.lib'],
          'version':          '0.1.1',
          'zip_safe':         False,
          'ext_package':      'pyhdfeos',
          'ext_modules':      ext_modules,
          'include_dirs':     [numpy.get_include()],
          'entry_points':     entry_points,
          'install_requires': install_requires,
          'license':          'MIT',
          'classifiers':      classifiers}
setup(**kwargs)
