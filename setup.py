from setuptools import setup
#from distutils.core import setup
import os
import re
import sys

from Cython.Build import cythonize

import pyhdfeos

kwargs = {'name': 'pyhdfeos',
          'description': 'Tools for accessing HDF-EOS grids',
          'long_description': open('README.md').read(),
          'author': 'John Evans',
          'author_email': 'john.g.evans.ne@gmail.com',
          'url': 'http://hdfeos.org',
          'packages': ['pyhdfeos', 'pyhdfeos.lib'],
          'version': '0.1.0rc3',
          'zip_safe':  False,
          'ext_modules': [pyhdfeos.lib.he4.ffi.verifier.get_extension(),
                          pyhdfeos.lib.he5.ffi.verifier.get_extension(),
                          cythonize("pyhdfeos/_som.pyx")[0]],
          'entry_points': {
              'console_scripts': ['hedump=pyhdfeos.command_line:dump_metadata'],
              },
          'license': 'MIT'}

install_requires = ['numpy>=1.8.0', 'cffi>=0.8.2']
if sys.hexversion < 0x03000000:
    install_requires.append('mock>=1.0.1')
kwargs['install_requires'] = install_requires

clssfrs = ["Programming Language :: Python",
           "Programming Language :: Python :: 2.7",
           "Programming Language :: Python :: 3.3",
           "Programming Language :: Python :: 3.4",
           "Programming Language :: Python :: Implementation :: CPython",
           "License :: OSI Approved :: MIT License",
           "Development Status :: 3 - Production/Alpha",
           "Operating System :: MacOS",
           "Operating System :: POSIX :: Linux",
           "Intended Audience :: Science/Research",
           "Intended Audience :: Information Technology",
           "Topic :: Software Development :: Libraries :: Python Modules"]
kwargs['classifiers'] = clssfrs

setup(**kwargs)
