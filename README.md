========
Overview
========

pyhdfeos is a wrapper for the HDF-EOS and HDF-EOS5 libraries.  


============
Installation
============

Prerequisites
=============
Check with your package manager to be sure that ```hdfeos5``` and
```hdfeos-devel``` packages are installed.

There is an issue with HDF-EOS5 version 1.15 that prevents it from being used
as-is with ```pyhdfeos```.

---
Mac
---

MacPorts
--------
MacPorts uses version HDF-EOS5 version 1.15, so it is vulnerable to the issue previously
described.

-----
Linux
-----

Linux Mint, Ubuntu 14.04 (other Ubuntu variants as well?)
---------------------------------------------------------
Linux Mint 17 uses HDF-EOS5 version 1.14, so it works out of the box.
Prerequisites include

* python-dev
* python-numpy
* python-cffi
* libhdf4-dev
* libhdf5-dev
* libhe5-hdfeos-dev
* libhdfeos-dev
* libgctp-dev

Fedora
------
RPM prerequisites include

* python-setuptools
* python-numpy
* python-cffi
* python-mock
* python-devel
* hdf-devel

There are no RPMs for HFDEOS, so the libraries must be compiled from source.
Assuming that the installation directory is ``/usr/local``.  The header files
must be manually installed.

Package Installation
====================

    python setup.py install --user
