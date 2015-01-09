"""
Support for HDF-EOS5 zonal average files.
"""
import collections
import os
import sys
import textwrap

import numpy as np

from .lib import he4, he5
from .core import EosFile, _EosField, DimensionMap


class ZonalAverageFile(EosFile):
    """
    Access to HDF-EOS zonal average files.

    Attributes
    ----------
    filename : str
        HDF-EOS5 zonal average file
    zas : dictionary
        collection of zonal average structures
    """
    def __init__(self, filename):
        EosFile.__init__(self)
        self.filename = filename
        self.zafid = he5.zaopen(filename)
        self._he = he5

        zalist = self._he.zainqza(filename)
        self.zas = collections.OrderedDict()
        for zaname in zalist:
            self.zas[zaname] = _ZonalAverage(self.filename, zaname, self._he)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        pass

    def __repr__(self):
        return "ZonalAverageFile('{0}')".format(self.filename)

    def __str__(self):
        orig_printoptions = np.get_printoptions()
        np.set_printoptions(precision=6)
        lst = ["{0}".format(os.path.basename(self.filename))]
        for za in self.zas.keys():
            lst.append(str(self.zas[za]))
        np.set_printoptions(**orig_printoptions)
        return '\n'.join(lst)

    def __del__(self):
        self._he.zaclose(self.zafid)


class _ZonalAverage(object):
    """
    Zonal Average object.
    """
    def __init__(self, filename, zaname, he_module):
        self.filename = filename
        self._he = he_module
        self.zafid = self._he.zaopen(filename)
        self.zaid = self._he.zaattach(self.zafid, zaname)
        self.zaname = zaname

        dimnames, dimlens = self._he.zainqdims(self.zaid)
        dims = [(k, v) for (k, v) in zip(dimnames, dimlens)]
        self.dims = collections.OrderedDict(dims)

        # collect the fieldnames
        fields, _, _ = self._he.zainquire(self.zaid)
        self.fields = collections.OrderedDict()
        for fieldname in fields:
            self.fields[fieldname] = _ZonalAverageVariable(fieldname,
                                                           self.zaid)

        attr_list = self._he.zainqattrs(self.zaid)
        self.attrs = collections.OrderedDict()
        for attr in attr_list:
            self.attrs[attr] = self._he.zareadattr(self.zaid, attr)

        attr_list = self._he.zainqgrpattrs(self.zaid)
        self.data_field_attrs = collections.OrderedDict()
        for attr in attr_list:
            val = self._he.zareadgrpattr(self.zaid, attr)
            self.data_field_attrs[attr] = val

    def __del__(self):
        self._he.zadetach(self.zaid)
        self._he.zaclose(self.zafid)

    def __str__(self):
        lst = ["Swath:  {0}".format(self.zaname)]
        lst.append("    Dimensions:")
        for dimname, dimlen in self.dims.items():
            lst.append("        {0}:  {1}".format(dimname, dimlen))

        lst.append("    Group Attributes:")
        fmt = "        {0}:  {1}"
        for attr in self.field_attrs.keys():
            lst.append(fmt.format(attr, self.field_attrs[attr]))

        lst.append("    Data Fields:")
        for field in self.datafields.keys():
            if sys.hexversion <= 0x03000000:
                textstr = str(self.datafields[field])
                field_lst = [(' ' * 8 + line) for line in textstr.split('\n')]
                lst.extend(field_lst)
            else:
                lst.append(textwrap.indent(str(self.datafields[field]),
                           ' ' * 8))

        lst.append("    Zonal Average Attributes:")
        for attr in self.attrs.keys():
            lst.append("        {0}:  {1}".format(attr, self.attrs[attr]))
        return '\n'.join(lst)


class _ZonalAverageVariable(_EosField):
    """
    """
    def __init__(self, fieldname, zaid):
        _EosField.__init__(self)
        self.fieldname = fieldname
        self.struct_id = zaid
        self._he = he5

        x = self._he.zainfo(self.struct_id, fieldname)
        self.shape, self.dtype, self.dimlist = x[0:3]

        self.attrs = collections.OrderedDict()
        attr_names = self._he.zainqlocattrs(self.struct_id, self.fieldname)
        for attrname in attr_names:
            self.attrs[attrname] = self._he.zareadlocattr(self.struct_id,
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
        return self._he.zareadfield(self.struct_id, self.fieldname,
                                    start, stride, edge)

