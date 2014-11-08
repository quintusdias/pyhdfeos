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

There is an issue with HDF-EOS5 version 5.1.15 that prevents it from being used
as-is with ```pyhdfeos```.

---
Mac
---

MacPorts
--------
MacPorts uses version 5.1.15, so it is vulnerable to the issue previously
described.

-----
Linux
-----

Linux Mint (other Ubuntu variants as well)
---------------------------------------------
Linux Mint 17 uses version 5.1.14, so it works out of the box.

Fedora 20
---------
There's no hdfeos or hdfeos5 RPMs specifically for Fedora, so you must compile
the libraries from source.  You may install it into ```/usr/local```.

Package Installation
====================

    python setup.py install --user
