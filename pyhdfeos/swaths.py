"""
Support for HDF-EOS and HDF-EOS5 swath files.
"""
import collections

from .lib import he4, he5, hdf
from .core import EosFile

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
                    attrs = self._hdf4_attrs(filename, swathname, fieldname, True)
                    self.swaths[swathname].geofields[fieldname].attrs = attrs
                for fieldname in self.swaths[swathname].datafields.keys():
                    attrs = self._hdf4_attrs(filename, swathname, fieldname, False)
                    self.swaths[swathname].datafields[fieldname].attrs = attrs

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
            self.geofields[fieldname] = _SwathVariable(fieldname, self.swathid, self._he)
        datafields, _, _ = self._he.swinqdatafields(self.swathid)
        self.datafields = collections.OrderedDict()
        for fieldname in datafields:
            self.datafields[fieldname] = _SwathVariable(fieldname, self.swathid, self._he)

        attr_list = self._he.swinqattrs(self.swathid)
        self.attrs = collections.OrderedDict()
        for attr in attr_list:
            self.attrs[attr] = self._he.swreadattr(self.swathid, attr)

    def __del__(self):
        self._he.swdetach(self.swathid)
        self._he.swclose(self.swfid)

class _SwathVariable(object):
    """
    """
    def __init__(self, fieldname, swathid, he_module):
        self.fieldname = fieldname
        self.swathid = swathid
        self.he_module = he_module
