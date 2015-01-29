"""
Required for grids, swaths.
"""
import collections
import textwrap
import sys

if sys.hexversion < 0x03000000:
    from itertools import ifilterfalse as filterfalse
else:
    from itertools import filterfalse

import numpy as np

from .lib import hdf, he2, he5

# swath dimension map definition
DimensionMap = collections.namedtuple('DimensionMap', ['offset', 'increment'])


class EosFile(object):
    """
    Attributes
    ----------
    filename : str
        path to HDF-EOS file
    """
    def __init__(self, filename):

        self.filename = filename

        # "_he" is an internal mechanism for storing the correct interface
        # to the correct HDF-EOS library.  Establish a placeholder here.
        self._he = None

        # Try to open the file as HDF4.  If we succeed, start up the SD and
        # vgroup interface to ease retrieval of SDS attributes.
        self._is_hdfeos2 = False
        if he2.ehheishe2(filename):
            self._fid = hdf.hopen(filename)
            self._sd_id = hdf.sdstart(filename)
            hdf.vstart(self._fid)
            self._is_hdfeos2 = True
        elif he5.ehheishe5(filename):
            pass
        else:
            msg = "{0} is neither HDF-EOS nor HDF-EOS5.".format(filename)
            raise RuntimeError(msg)

    def __del__(self):

        # Close the HDF4 raw file IDs
        if self._is_hdfeos2:
            hdf.vend(self._fid)
            hdf.sdend(self._sd_id)
            hdf.hclose(self._fid)

    def _position_to_hdf4_vgroup(self, eos_vg_name, geolocation=False):
        """
        locate the proper HDF4 vgroup

        Locate the proper HDF4 vgroup that corresponds to the HDF-EOS
        grid/swath name.

        Parameters
        ----------
        eos_vg_name : str
            either the name of an HDF-EOS grid or swath
        geolocation : bool
            If true, then the field being sought is a geolocation field,
            not a data field.  This is useful only for swaths.
        """
        grid_ref = hdf.vfind(self._fid, eos_vg_name)
        grid_vg = hdf.vattach(self._fid, grid_ref)

        members = hdf.vgettagrefs(grid_vg)

        # iterate thru all the vgroup members until we find the vgroup with the
        # name matching that of the grid/swath
        for tag_i, ref_i in members:
            if tag_i == hdf.DFTAG_VG:
                # Descend into a vgroup if we find it.
                vg0 = hdf.vattach(self._fid, ref_i)
                name = hdf.vgetname(vg0)
                if (((hasattr(self, '_swfid')) and
                     (geolocation and name == 'Geolocation Fields'))):
                    self._hdf4_vgroup_ref = ref_i
                    hdf.vdetach(vg0)
                    return
                elif (((hasattr(self, '_swfid')) and
                       (not geolocation and name == 'Data Fields'))):
                    self._hdf4_vgroup_ref = ref_i
                    hdf.vdetach(vg0)
                    return
                elif hasattr(self, '_gdfid') and (name == 'Data Fields'):
                    self._hdf4_vgroup_ref = ref_i
                    hdf.vdetach(vg0)
                    return
                hdf.vdetach(vg0)

        hdf.vdetach(grid_vg)

    def _get_sds_attributes(self, fieldname):

        vgroup = hdf.vattach(self._fid, self._hdf4_vgroup_ref)

        attrs = collections.OrderedDict([])
        df_members = hdf.vgettagrefs(vgroup)
        for tag_j, ref_j in df_members:
            if tag_j == hdf.DFTAG_NDG:
                # SDS dataset.
                idx = hdf.sdreftoindex(self._sd_id, ref_j)
                sds_id = hdf.sdselect(self._sd_id, idx)
                name, dims, dtype, nattrs = hdf.sdgetinfo(sds_id)
                if name == fieldname:
                    alst = []
                    for k in range(nattrs):
                        info = hdf.sdattrinfo(sds_id, k)
                        name = info[0]
                        value = hdf.sdreadattr(sds_id, k)
                        alst.append((name, value))
                    attrs = collections.OrderedDict(alst)
                hdf.sdendaccess(sds_id)

        hdf.vdetach(vgroup)
        return attrs


class _EosStruct(object):
    """
    Abstract superclass for grid, swath, zonal average structures.
    """
    def __init__(self):
        pass

    def _textwrap(self, text, nspace):
        """
        Indent text a certain number of spaces.

        Parameters
        ----------
        text : str
            Possibly multi-line text to be indented.
        nspace : int
            Number of spaces to indent.
        """
        if sys.hexversion <= 0x03000000:
            lst = [(' ' * nspace + line) for line in text.split('\n')]
            indented_text = '\n'.join(lst)
        else:
            indented_text = textwrap.indent(text, ' ' * nspace)
        return indented_text

    def _format_fields(self, title, fields):
        """
        Apply formatting to fields.

        Parameters
        ----------
        title : str
            Something like "Swath Data Field Group Attributes"
        fields : dict
            All fields for a particular entity, such as a Swath
            Geolocation Field Group.
        """
        title = title + ':'

        lst = []
        for field in fields.keys():
            lst.append(str(fields[field]))
        text = '\n'.join(lst)
        text = self._textwrap(text, 4)

        return title + '\n' + text

    def _format_dimensions(self):
        """
        Apply formatting to the dimensions.
        """
        text = "Dimensions:"

        lst = []
        for dimname, dimlen in self.dims.items():
            lst.append("{0}:  {1}".format(dimname, dimlen))
        text = '\n'.join(lst)
        text = self._textwrap(text, 4)

        return 'Dimensions:' + '\n' + text

    def _format_attributes(self, title, attrs):
        """
        Apply formatting to attribute group.

        Slap a title on top of an attribute name/value listing.

        Parameters
        ----------
        title : str
            Something like "Swath Data Field Group Attributes"
        attrs : dict
            All attributes for a particular entity, such as a Swath
            Data Field Group.
        """
        title = title + ':'
        if len(attrs) == 0:
            return title

        fmt_reg = "{0}:  {1}"
        fmt_flt = "{0}:  {1:.8f}"

        attrs_lst = []
        for attr in attrs.keys():
            if isinstance(attrs[attr], np.float32):
                text = fmt_flt.format(attr, attrs[attr])
            else:
                text = fmt_reg.format(attr, attrs[attr])
            attrs_lst.append(text)

        attrs_text = '\n'.join(attrs_lst)
        attrs_text = self._textwrap(attrs_text, 4)

        text = title + '\n' + attrs_text
        return text


class _EosField(object):
    """
    Abstract superclass for grid, swath, zonal average fields.
    """
    def __init__(self):
        pass

    def __getitem__(self, index):
        ndims = len(self.shape)

        # Set up defaults.
        start = np.zeros(ndims)
        stride = np.ones(ndims)
        edge = list(self.shape)

        if isinstance(index, int):
            # Retrieve a row, unless this is a 1D array.  If that is the case,
            # then return a scalar.
            start[0] = index
            stride[0] = 1
            edge[0] = 1
            data = self._readfield(start, stride, edge)

            # If a 1D dataset, just return a scalar.
            if len(self.shape) == 1:
                data = data[0]
            else:
                # Squeeze out the row dimension, i.e. no singleton dimensions.
                data = np.squeeze(data, axis=0)
            return data

        if index is Ellipsis:
            # Case of [...]
            # Handle it below.
            return self.__getitem__(slice(None, None, None))

        if isinstance(index, slice):
            if (((index.start is None) and
                 (index.stop is None) and
                 (index.step is None))):
                # Case of [:].  Read all of the data.
                return self._readfield(start, stride, edge)

            msg = "Single slice argument integer is only legal if ':'"
            raise RuntimeError(msg)

        if isinstance(index, tuple) and any(x is Ellipsis for x in index):
            # Remove the first ellipsis we find and try again.
            newindex = self._resolve_first_ellipsis(index)
            return self.__getitem__(newindex)

        if isinstance(index, tuple) and any(isinstance(x, int) for x in index):
            # Find the first such integer argument, replace it with a slice.
            lst = list(index)
            predicate = lambda x: not isinstance(x[1], int)
            g = filterfalse(predicate, enumerate(index))
            idx = next(g)[0]
            lst[idx] = slice(index[idx], index[idx] + 1)
            newindex = tuple(lst)

            # Invoke array-based slicing again, as there may be additional
            # integer argument remaining.
            data = self.__getitem__(newindex)

            # Reduce dimensionality in the scalar dimension.
            data = np.squeeze(data, axis=idx)
            return data

        # Assuming pargs is a tuple of slices from now on.
        # This is the workhorse section for the general case.
        for j in range(len(index)):

            if index[j].start is not None:
                start[j] = index[j].start

            if index[j].step is not None:
                stride[j] = index[j].step

            if index[j].stop is None:
                edge[j] = np.floor((self.shape[j] - start[j]) / stride[j])
            else:
                edge[j] = np.floor((index[j].stop - start[j]) / stride[j])

        return self._readfield(start, stride, edge)

    def _resolve_first_ellipsis(self, index):
        """
        Find the first ellipsis index argument and resolve it into a slice.

        Parameters
        ----------
        index : tuple
            index argument passed to __getitem__ method, contains at least
            one ellipsis
        """
        newindex = []
        first_ellipsis = True
        for j, idx in enumerate(index):
            if idx is Ellipsis and first_ellipsis:
                newindex.append(slice(0, self.shape[j]))
                first_ellipsis = False
            else:
                newindex.append(idx)

        newindex = tuple(newindex)
        return newindex
