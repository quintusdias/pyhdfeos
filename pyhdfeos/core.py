"""
Required for grids, swaths.
"""
import collections
import sys

if sys.hexversion < 0x03000000:
    from itertools import ifilterfalse as filterfalse
else:
    from itertools import filterfalse

import numpy as np

from .lib import hdf

# swath dimension map definition
DimensionMap = collections.namedtuple('DimensionMap', ['offset', 'increment'])


class EosFile(object):
    """
    """
    def __init__(self):
        # "_he" is an internal mechanism for storing the correct interface
        # to the correct HDF-EOS library.  Establish a placeholder here.
        self._he = None
        pass

    def _hdf4_attrs(self, filename, eos_vg_name, fieldname, geolocation=False):
        """
        Retrieve field attributes using HDF4 interface.

        Parameters
        ----------
        filename : str
            HDF-EOS file
        eos_vg_name : str
            either the name of an HDF-EOS grid or swath
        fieldname : str
            field being sought, if it is a merged field, it will not be
            found
        geolocation : bool
            If true, then the field being sought is a geolocation field,
            not a data field.  This is useful only for swaths.
        """

        attrs = None

        fid = hdf.hopen(filename)
        sd_id = hdf.sdstart(filename)
        hdf.vstart(fid)

        grid_ref = hdf.vfind(fid, eos_vg_name)
        grid_vg = hdf.vattach(fid, grid_ref)

        members = hdf.vgettagrefs(grid_vg)
        for tag_i, ref_i in members:
            if tag_i == hdf.DFTAG_VG:
                # Descend into a vgroup if we find it.
                vg0 = hdf.vattach(fid, ref_i)
                name = hdf.vgetname(vg0)
                if (((hasattr(self, '_swfid')) and
                     ((geolocation and name == 'Geolocation Fields') or
                      (not geolocation and name == 'Data Fields')))):
                    attrs = self.collect_attrs_from_sds_in_vgroup(sd_id,
                                                                  vg0,
                                                                  fieldname)
                elif hasattr(self, '_gdfid') and name == 'Data Fields':
                    attrs = self.collect_attrs_from_sds_in_vgroup(sd_id,
                                                                  vg0,
                                                                  fieldname)
                hdf.vdetach(vg0)

        hdf.vdetach(grid_vg)

        hdf.vend(fid)
        hdf.sdend(sd_id)
        hdf.hclose(fid)

        if attrs is None:
            # No attributes.
            attrs = collections.OrderedDict()
        return attrs

    def collect_attrs_from_sds_in_vgroup(self, sd_id, vgroup, fieldname):

        attrs = None
        df_members = hdf.vgettagrefs(vgroup)
        for tag_j, ref_j in df_members:
            if tag_j == hdf.DFTAG_NDG:
                # SDS dataset.
                idx = hdf.sdreftoindex(sd_id, ref_j)
                sds_id = hdf.sdselect(sd_id, idx)
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

        return attrs


class _EosField(object):
    """
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
            for j in range(1, ndims):
                start[j] = 0
                stride[j] = 1
                edge[j] = self.shape[j]
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
            # Remove the first ellipsis we find.
            newindex = []
            first_ellipsis = True
            for j, idx in enumerate(index):
                if idx is Ellipsis and first_ellipsis:
                    newindex.append(slice(0, self.shape[j]))
                    first_ellipsis = False
                else:
                    newindex.append(idx)

            # Run once again because it is possible that there's another
            # Ellipsis object.
            newindex = tuple(newindex)
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
