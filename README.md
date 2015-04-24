========
PYHDFEOS
========

![ScreenShot](https://github.com/quintusdias/pyhdfeos/blob/issue37/docs/source/misr.png)

pyhdfeos is a wrapper for the HDF-EOS and HDF-EOS5 libraries.  


============
Installation
============

Prerequisites
=============
Python versions 2.7 and 3.4 are supported on Linux and Mac.  Required python
packages include

    * cffi
    * cython
    * numpy
    * numpy-devel

You must have both the hdf4 and hdf5 libraries installed, along with their
respective development packages.  It is not necessary to install HDF-EOS or 
HDF-EOS5, as that functionality is provided by this package.

Quick Start
===========
    * git clone https://github.com/quintusdias/pyhdfeos.git
    * git checkout devel
    * python setup.py develop

::
    
    >>> from pyhdfeos import GridFile
    >>> gdf = GridFile('tests/data/Grid.h5')
    >>> lat, lon = gdf.grids['UTMGrid'][:]
