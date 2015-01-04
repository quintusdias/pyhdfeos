"""
Support for HDF-EOS and HDF-EOS5 swath files.
"""
import collections
import os
import sys
import textwrap

import numpy as np

from .lib import he4, he5
from .core import EosFile, _EosField, DimensionMap


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
        EosFile.__init__(self, filename)
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
            if self._is_hdf4:
                # Inquire about hdf4 attributes using SD interface
                self._position_to_hdf4_vgroup(swathname, True)
                for fieldname in self.swaths[swathname].geofields.keys():
                    attrs = self._get_sds_attributes(fieldname)
                    self.swaths[swathname].geofields[fieldname].attrs = attrs

                self._position_to_hdf4_vgroup(swathname, False)
                for fieldname in self.swaths[swathname].datafields.keys():
                    attrs = self._get_sds_attributes(fieldname)
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
        for j in range(len(idxsizes)):
            geodim, datadim = dimmap_names[j].split('/')
            index = self._he.swidxmapinfo(self.swathid, geodim, datadim)
            self.idxmaps[dimmap_names[j]] = index

        attr_list = self._he.swinqattrs(self.swathid)
        self.attrs = collections.OrderedDict()
        for attr in attr_list:
            self.attrs[attr] = self._he.swreadattr(self.swathid, attr)

        if hasattr(self._he, 'swinqgrpattrs'):
            attr_list = self._he.swinqgrpattrs(self.swathid)
            self.data_field_attrs = collections.OrderedDict()
            for attr in attr_list:
                val = self._he.swreadgrpattr(self.swathid, attr)
                self.data_field_attrs[attr] = val

        if hasattr(self._he, 'swinqgeogrpattrs'):
            attr_list = self._he.swinqgeogrpattrs(self.swathid)
            self.geo_field_attrs = collections.OrderedDict()
            for attr in attr_list:
                val = self._he.swreadgeogrpattr(self.swathid, attr)
                self.geo_field_attrs[attr] = val

    def __del__(self):
        self._he.swdetach(self.swathid)
        self._he.swclose(self.swfid)

    def __str__(self):
        lst = ["Swath:  {0}".format(self.swathname)]
        lst.append("    Dimensions:")
        for dimname, dimlen in self.dims.items():
            lst.append("        {0}:  {1}".format(dimname, dimlen))

        lst.append("    Dimension Maps:")
        for name, map in self.dimmaps.items():
            msg = "        {0}:  offset={1}, increment={2}"
            msg = msg.format(name, map.offset, map.increment)
            lst.append(msg)

        lst.append("    Index Maps:")
        for name, idx in self.idxmaps.items():
            msg = "        {0}:  index={1}".format(name, idx)
            lst.append(msg)

        if hasattr(self._he, 'swinqgeogrpattrs'):
            lst.append("    Geolocation Group Attributes:")
            fmt = "        {0}:  {1}"
            for attr in self.geo_field_attrs.keys():
                lst.append(fmt.format(attr, self.geo_field_attrs[attr]))

        lst.append("    Geolocation Group Fields:")
        for field in self.geofields.keys():
            if sys.hexversion <= 0x03000000:
                textstr = str(self.geofields[field])
                field_lst = [(' ' * 8 + line) for line in textstr.split('\n')]
                lst.extend(field_lst)
            else:
                lst.append(textwrap.indent(str(self.geofields[field]),
                           ' ' * 8))

        if hasattr(self._he, 'swinqgrpattrs'):
            lst.append("    Data Group Attributes:")
            fmt = "        {0}:  {1}"
            for attr in self.data_field_attrs.keys():
                lst.append(fmt.format(attr, self.data_field_attrs[attr]))

        lst.append("    Data Group Fields:")
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


class _SwathVariable(_EosField):
    """
    """
    def __init__(self, fieldname, swathid, he_module):
        _EosField.__init__(self)
        self.fieldname = fieldname
        self.struct_id = swathid
        self._he = he_module

        x = self._he.swfieldinfo(self.struct_id, fieldname)
        self.shape, self.dtype, self.dimlist = x[0:3]

        # HDFEOS5 only.
        self.attrs = collections.OrderedDict()
        if hasattr(self._he, 'swinqlocattrs'):
            attr_names = self._he.swinqlocattrs(self.struct_id, self.fieldname)
            for attrname in attr_names:
                self.attrs[attrname] = self._he.swreadlocattr(self.struct_id,
                                                              self.fieldname,
                                                              attrname)

    def __str__(self):
        dimstr = ", ".join(self.dimlist)
        dtype_str = str(self.dtype).split('.')[1].split("'")[0]
        lst = ["{0} {1}[{2}]:".format(dtype_str, self.fieldname, dimstr)]

        for name, value in self.attrs.items():
            lst.append("    {0}:  {1}".format(name, value))
        return '\n'.join(lst)

    def _readfield(self, start, stride, edge):
        return self._he.swreadfield(self.struct_id, self.fieldname,
                                    start, stride, edge)
