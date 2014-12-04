from setuptools import setup
import os
import re
import sys

from Cython.Build import cythonize
import numpy

# We need to locate libGctp (libgctp if on a debian variant) in order to 
# compile the som grid extension module.
try:
    include_dirs = os.environ['INCLUDE_DIRS'].split(':')
except KeyError:
    include_dirs = ['/usr/include', '/usr/local/include', '/opt/local/include']

try:
    library_dirs = os.environ['LIBRARY_DIRS'].split(":")
except KeyError:
    library_dirs = ['/usr/lib', '/usr/local/lib', '/opt/local/lib']

def locate_gctp(library_dirs):
    """
    Debian systems (including Mint) have libGctp.  Most other linux systems
    plus macports have libgctp.  Need to find out which one to use.
    """
    libs = ['gctp', 'Gctp']
    suffix_list = ['a', 'so', 'dylib', 'dll']
    for library_dir in library_dirs:
        for libname in libs:
            for suffix in suffix_list:
                path = os.path.join(library_dir, 'lib' + libname + '.' + suffix)
                if os.path.exists(path):
                    return libname
    return None

true_gctp_lib = locate_gctp(library_dirs)
if true_gctp_lib is None:
    msg = "Could not locate gctp library.  Please specify a location with "
    msg += "the LIBRARY_DIRS environment variable as specified in README.md."
    raise RuntimeError(msg)

import pyhdfeos

ext_modules = [pyhdfeos.lib.he4.ffi.verifier.get_extension(),
               pyhdfeos.lib.he5.ffi.verifier.get_extension()]

from distutils.extension import Extension
cythonize("pyhdfeos/_som.pyx")
e = Extension("pyhdfeos/_som", ["pyhdfeos/_som.c"],
        include_dirs = include_dirs,
        libraries    = [true_gctp_lib],
        library_dirs = library_dirs)
ext_modules.append(e)

install_requires = ['numpy>=1.8.0', 'cffi>=0.8.2', 'pyhdf>=0.9']
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
