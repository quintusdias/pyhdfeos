"""
Support for HDF-EOS and HDF-EOS5 swath files.
"""
import collections
import os
import sys
import textwrap

import numpy as np

from .lib import he4, he5
from .core import EosFile, DimensionMap


class SwathFile(EosFile):
    """
    Access to HDF-EOS swath files.

    Attributes
    ----------
    filename : str
        HDF-EOS2 or HDF-EOS5 swath file
    swaths : dictionary
        collection of swaths
    """
    def __init__(self, filename):
        EosFile.__init__(self)
        self.filename = filename
        try:
            self.swfid = he4.swopen(filename)
            self._he = he4
        except IOError:
            # try hdf5
            self.swfid = he5.swopen(filename)
            self._he = he5

        swathlist = self._he.swinqswath(filename)
        self.swaths = collections.OrderedDict()
        for swathname in swathlist:
            self.swaths[swathname] = _Swath(self.filename, swathname, self._he)
            if not hasattr(self._he, 'swinqlocattrs'):
                # Inquire about hdf4 attributes using SD interface
                for fieldname in self.swaths[swathname].geofields.keys():
                    attrs = self._hdf4_attrs(filename, swathname, fieldname,
                                             True)
                    self.swaths[swathname].geofields[fieldname].attrs = attrs
                for fieldname in self.swaths[swathname].datafields.keys():
                    attrs = self._hdf4_attrs(filename, swathname, fieldname,
                                             False)
                    self.swaths[swathname].datafields[fieldname].attrs = attrs

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        pass

    def __repr__(self):
        return "SwathFile('{0}')".format(self.filename)

    def __str__(self):
        orig_printoptions = np.get_printoptions()
        np.set_printoptions(precision=6)
        lst = ["{0}".format(os.path.basename(self.filename))]
        for swath in self.swaths.keys():
            lst.append(str(self.swaths[swath]))
        np.set_printoptions(**orig_printoptions)
        return '\n'.join(lst)

    def __del__(self):
        self._he.swclose(self.swfid)


class _Swath(object):
    """
    Swath object.
    """
    def __init__(self, filename, swathname, he_module):
        self.filename = filename
        self._he = he_module
        self.swfid = self._he.swopen(filename)
        self.swathid = self._he.swattach(self.swfid, swathname)
        self.swathname = swathname

        dimnames, dimlens = self._he.swinqdims(self.swathid)
        dims = [(k, v) for (k, v) in zip(dimnames, dimlens)]
        self.dims = collections.OrderedDict(dims)

        # collect the fieldnames
        geofields, _, _ = self._he.swinqgeofields(self.swathid)
        self.geofields = collections.OrderedDict()
        for fieldname in geofields:
            self.geofields[fieldname] = _SwathVariable(fieldname,
                                                       self.swathid,
                                                       self._he)
        datafields, _, _ = self._he.swinqdatafields(self.swathid)
        self.datafields = collections.OrderedDict()
        for fieldname in datafields:
            self.datafields[fieldname] = _SwathVariable(fieldname,
                                                        self.swathid,
                                                        self._he)

        dimmap_names, offsets, increments = self._he.swinqmaps(self.swathid)
        self.dimmaps = collections.OrderedDict()
        for j in range(len(offsets)):
            dimmap = DimensionMap(offset=offsets[j], increment=increments[j])
            self.dimmaps[dimmap_names[j]] = dimmap

        dimmap_names, idxsizes = self._he.swinqidxmaps(self.swathid)
        self.idxmaps = collections.OrderedDict()
        for j in range(len(dimmap_names)):
            geodim, datadim = dimmap_names[j].split('/')
            index = self._he.swidxmapinfo(self.swathid, geodim, datadim)
            self.idxmaps[dimmap_names[j]] = index

        attr_list = self._he.swinqattrs(self.swathid)
        self.attrs = collections.OrderedDict()
        for attr in attr_list:
            self.attrs[attr] = self._he.swreadattr(self.swathid, attr)

    def __del__(self):
        self._he.swdetach(self.swathid)
        self._he.swclose(self.swfid)

    def __str__(self):
        lst = ["Swath:  {0}".format(self.swathname)]
        lst.append("    Dimensions:")
        for dimname, dimlen in self.dims.items():
            lst.append("        {0}:  {1}".format(dimname, dimlen))

        lst.append("    Geolocation Fields:")
        for field in self.geofields.keys():
            if sys.hexversion <= 0x03000000:
                textstr = str(self.geofields[field])
                field_lst = [(' ' * 8 + line) for line in textstr.split('\n')]
                lst.extend(field_lst)
            else:
                lst.append(textwrap.indent(str(self.geofields[field]),
                           ' ' * 8))

        lst.append("    Dimension Maps:")
        for name, map in self.dimmaps.items():
            msg = "        {0}:  offset={1}, increment={2}"
            msg = msg.format(name, map.offset, map.increment)
            lst.append(msg)

        lst.append("    Index Maps:")
        for name, idx in self.idxmaps.items():
            msg = "        {0}:  index={1}".format(name, idx)
            lst.append(msg)

        lst.append("    Data Fields:")
        for field in self.datafields.keys():
            if sys.hexversion <= 0x03000000:
                textstr = str(self.datafields[field])
                field_lst = [(' ' * 8 + line) for line in textstr.split('\n')]
                lst.extend(field_lst)
            else:
                lst.append(textwrap.indent(str(self.datafields[field]),
                           ' ' * 8))

        lst.append("    Swath Attributes:")
        for attr in self.attrs.keys():
            lst.append("        {0}:  {1}".format(attr, self.attrs[attr]))
        return '\n'.join(lst)


class _SwathVariable(object):
    """
    """
    def __init__(self, fieldname, swathid, he_module):
        self.fieldname = fieldname
        self.swathid = swathid
        self._he = he_module

        x = self._he.swfieldinfo(self.swathid, fieldname)
        self.shape, self.dtype, self.dimlist = x[0:3]

    def __str__(self):
        dimstr = ", ".join(self.dimlist)
        lst = ["{0}[{1}]:".format(self.fieldname, dimstr)]

        for name, value in self.attrs.items():
            lst.append("    {0}:  {1}".format(name, value))
        return '\n'.join(lst)
