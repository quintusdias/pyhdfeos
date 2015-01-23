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
            self._swfid = he4.swopen(filename)
            self._he = he4
        except IOError:
            # try hdf5
            self._swfid = he5.swopen(filename)
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
        self._he.swclose(self._swfid)


class _Swath(object):
    """
    Swath object.

    Attributes
    ----------
    attrs : dictionary
        collection of swath attributes
    dimmaps : dictionary
        collections of dimension maps defined in the swath
    dims : list
        list of dimension names and extents defined in the swath
    name : str
        name of swath
    datafield_attrs : dictionary
        collection of data field group attributes (empty in HDF4)
    datafields : dictionary
        collection of data field objects defined in the swath
    geofield_attrs : dictionary
        collection of geolocation field group attributes (empty in HDF4)
    geofields : dictionary
        collection of geolocation field objects defined in the swath
    idxmaps : dictionary
        collections of index maps defined in the swath
    """
    def __init__(self, filename, swathname, he_module):
        self._filename = filename
        self._he = he_module
        self._swfid = self._he.swopen(filename)
        self._swathid = self._he.swattach(self._swfid, swathname)
        self.name = swathname

        dimnames, dimlens = self._he.swinqdims(self._swathid)
        dims = [(k, v) for (k, v) in zip(dimnames, dimlens)]
        self.dims = collections.OrderedDict(dims)

        # collect the fieldnames
        geofields, _, _ = self._he.swinqgeofields(self._swathid)
        self.geofields = collections.OrderedDict()
        for fieldname in geofields:
            self.geofields[fieldname] = _SwathVariable(fieldname,
                                                       self._swathid,
                                                       self._he)
        datafields, _, _ = self._he.swinqdatafields(self._swathid)
        self.datafields = collections.OrderedDict()
        for fieldname in datafields:
            self.datafields[fieldname] = _SwathVariable(fieldname,
                                                        self._swathid,
                                                        self._he)

        dimmap_names, offsets, increments = self._he.swinqmaps(self._swathid)
        self.dimmaps = collections.OrderedDict()
        for j in range(len(offsets)):
            dimmap = DimensionMap(offset=offsets[j], increment=increments[j])
            self.dimmaps[dimmap_names[j]] = dimmap

        dimmap_names, idxsizes = self._he.swinqidxmaps(self._swathid)
        self.idxmaps = collections.OrderedDict()
        for j in range(len(idxsizes)):
            geodim, datadim = dimmap_names[j].split('/')
            index = self._he.swidxmapinfo(self._swathid, geodim, datadim)
            self.idxmaps[dimmap_names[j]] = index

        attr_list = self._he.swinqattrs(self._swathid)
        self.attrs = collections.OrderedDict()
        for attr in attr_list:
            self.attrs[attr] = self._he.swreadattr(self._swathid, attr)

        if hasattr(self._he, 'swinqgrpattrs'):
            # data field group attributes are HDF-EOS5 only
            attr_list = self._he.swinqgrpattrs(self._swathid)
            self.datafield_attrs = collections.OrderedDict()
            for attr in attr_list:
                val = self._he.swreadgrpattr(self._swathid, attr)
                self.datafield_attrs[attr] = val

            # geolocation field group attributes are HDF-EOS5 only
            attr_list = self._he.swinqgeogrpattrs(self._swathid)
            self.geofield_attrs = collections.OrderedDict()
            for attr in attr_list:
                val = self._he.swreadgeogrpattr(self._swathid, attr)
                self.geofield_attrs[attr] = val

    def __del__(self):
        self._he.swdetach(self._swathid)
        self._he.swclose(self._swfid)

    def _textwrap(self, text, nspace):
        """
        """
        if sys.hexversion <= 0x03000000:
            lst = [(' ' * nspace + line) for line in text.split('\n')]
            indented_text = '\n'.join(lst)
        else:
            indented_text = textwrap.indent(text, ' ' * nspace)
        return indented_text
	
    def _format_attributes(self, title, attrs):
        """
        """
        title = title + ':'
        if len(attrs) == 0:
            return [self._textwrap(title, 4)]

        fmt_reg = "{0}:  {1}"
        fmt_flt = "{0}:  {1:.8f}"

        attrs_text = ''
        for attr in attrs.keys():
            if isinstance(attrs[attr], np.float32):
                attrs_text += fmt_flt.format(attr, attrs[attr])
            else:
                attrs_text += fmt_reg.format(attr, attrs[attr])
        attrs_text = self._textwrap(attrs_text, 4)

        text = title + '\n' + attrs_text
        text = self._textwrap(text, 4)
        return text.split('\n')

    def __str__(self):
        lst = ["Swath:  {0}".format(self.name)]
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
            lst.extend(self._format_attributes('Geolocation Group Attributes',
                                               self.geofield_attrs))

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
            lst.extend(self._format_attributes('Data Group Attributes',
                                               self.datafield_attrs))

        lst.append("    Data Group Fields:")
        for field in self.datafields.keys():
            if sys.hexversion <= 0x03000000:
                textstr = str(self.datafields[field])
                field_lst = [(' ' * 8 + line) for line in textstr.split('\n')]
                lst.extend(field_lst)
            else:
                lst.append(textwrap.indent(str(self.datafields[field]),
                           ' ' * 8))

        lst.extend(self._format_attributes('Swath Attributes',
                                           self.attrs))
        return '\n'.join(lst)


class _SwathVariable(_EosField):
    """
    Swath field object (data, dimensions, attributes)

    Attributes
    ----------
    fieldname : str
        name of field
    shape : tuple
        dimension extents of field
    dtype : numpy datatype
        numpy datatype class that corresponds to the in-file datatype
    dims : list
        list of dimension names that define the field extents
    """
    def __init__(self, fieldname, swathid, he_module):
        _EosField.__init__(self)
        self.fieldname = fieldname
        self._id = swathid
        self._he = he_module

        x = self._he.swfieldinfo(self._id, fieldname)
        self.shape, self.dtype, self.dims = x[0:3]

        # HDFEOS5 only.
        self.attrs = collections.OrderedDict()
        if hasattr(self._he, 'swinqlocattrs'):
            attr_names = self._he.swinqlocattrs(self._id, self.fieldname)
            for attrname in attr_names:
                self.attrs[attrname] = self._he.swreadlocattr(self._id,
                                                              self.fieldname,
                                                              attrname)

    def __str__(self):
        dimstr = ", ".join(self.dims)
        dtype_str = str(self.dtype).split('.')[1].split("'")[0]
        lst = ["{0} {1}[{2}]:".format(dtype_str, self.fieldname, dimstr)]

        for name, value in self.attrs.items():
            lst.append("    {0}:  {1}".format(name, value))
        return '\n'.join(lst)

    def _readfield(self, start, stride, edge):
        return self._he.swreadfield(self._id, self.fieldname,
                                    start, stride, edge)
