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

Both the HDF-EOS and HDF-EOS5 libraries are required.  On some
systems, such as Linux Mint, these are readily available through
the system package manager, just make sure you have the devel
packages installed as well.  On such a system, the install command is just::

   python setup.py install --user 

Other systems such as OpenSUSE do not provide packages for HDF-EOS or HDF-EOS5,
so they must be compiled from source.

Be advised, however, that there is a bug with HDF-EOS version 1.15
that prevents it from being used as-is with ```pyhdfeos```.  You
can patch it by editing ```src/EHapi.c``` and delete line 11545.

Ideally, you would install the libraries and header files into
```/usr/local```, but if that is not possible, you would need to
augment the search path when installing pyhdfeos with ```INCLUDE_DIRS```
and ```LIBRARY_DIRS``` environment variables, something like

    export INCLUDE_DIRS=/path/to/hdfeos/include
    export LIBRARY_DIRS=/path/to/hdfeos/lib
    python setup.py install --user 

Macports provides packages for both HDF-EOS and HDF-EOS5, but the HDF-EOS5 port
has the bug described above, so you should compile HDF-EOS5 from source.
