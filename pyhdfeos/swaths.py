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
                for fieldname in self.swaths[swathname].fields.keys():
                    attrs = self._hdf4_attrs(filename, swathname, fieldname)
                    self.swaths[swathname].fields[fieldname].attrs = attrs

class _Swath(object):
    """
    Swath object.
    """
    def __init__(self, filename, swathname, he_module):
        self.filename = filename

