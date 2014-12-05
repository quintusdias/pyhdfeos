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

Both HDF-EOS and HDF-EOS5 are required.  On some systems, such as Linux Mint,
this is pretty easy, just make sure you have the devel packages installed as
well.

Other systems such as OpenSUSE do not provide packages for HDF-EOS or HDF-EOS5,
so they must be compiled from source.  You should install the libraries (and
the header files) into ```/usr/local```.  Be advised, however, that there is
a bug with HDF-EOS version 1.15 that prevents it from being used as-is with
```pyhdfeos```.  You can patch it by editing ```src/EHapi.c``` and delete line
11545.

Macports provides packages for both HDF-EOS and HDF-EOS5, but the HDF-EOS5 port
has the bug described above, so you should compile HDF-EOS5 from source.
