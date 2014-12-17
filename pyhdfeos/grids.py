"""
Support for HDF-EOS and HDF-EOS5 grid files
"""

import collections
import os
import sys
import textwrap

if sys.hexversion < 0x03000000:
    from itertools import ifilterfalse as filterfalse
else:
    from itertools import filterfalse

import numpy as np

from .lib import he4, he5, hdf
from . import _som


class _GridVariable(object):
    """
    Grid field object (data, dimensions, attributes)
    """
    def __init__(self, gridid, fieldname, he_module):
        self.gridid = gridid
        self.fieldname = fieldname
        self._he = he_module

        x = self._he.gdfieldinfo(self.gridid, fieldname)
        self.shape, self.ntype, self.dimlist = x[0:3]

        # HDFEOS5 only.
        self.attrs = collections.OrderedDict()
        if hasattr(self._he, 'gdinqlocattrs'):
            attr_names = self._he.gdinqlocattrs(self.gridid, self.fieldname)
            for attrname in attr_names:
                self.attrs[attrname] = self._he.gdreadlocattr(self.gridid,
                                                              self.fieldname,
                                                              attrname)

    def __str__(self):
        dimstr = ", ".join(self.dimlist)
        lst = ["{0}[{1}]:".format(self.fieldname, dimstr)]

        for name, value in self.attrs.items():
            lst.append("    {0}:  {1}".format(name, value))
        return '\n'.join(lst)

    def __getitem__(self, index):
        ndims = len(self.shape)

        # Set up defaults.
        start = np.zeros(ndims)
        stride = np.ones(ndims)
        edge = list(self.shape)

        if isinstance(index, int):
            # Retrieve a row.
            start[0] = index
            stride[0] = 1
            edge[0] = 1
            for j in range(1, ndims):
                start[j] = 0
                stride[j] = 1
                edge[j] = self.shape[j]
            data = self._he.gdreadfield(self.gridid, self.fieldname,
                                        start, stride, edge)

            # Reduce dimensionality in the row dimension.
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
                return self._he.gdreadfield(self.gridid, self.fieldname,
                                            start, stride, edge)

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

        return self._he.gdreadfield(self.gridid, self.fieldname,
                                    start, stride, edge)


class _Grid(object):
    """
    Grid object, concerned only with coordinates of HDF-EOS grids.

    Attributes
    ----------
    projcode : scalar
    """
    def __init__(self, filename, gridname, he_module):
        self.filename = filename
        self._he = he_module
        self.gdfid = self._he.gdopen(filename)
        self.gridid = self._he.gdattach(self.gdfid, gridname)
        self.gridname = gridname

        dimnames, dimlens = self._he.gdinqdims(self.gridid)
        dims = [(k, v) for (k, v) in zip(dimnames, dimlens)]
        self.dims = collections.OrderedDict(dims)

        _tuple = self._he.gdgridinfo(self.gridid)
        self.xdimsize, self.ydimsize, self.upleft, self.lowright = _tuple
        if 'XDim' not in self.dims:
            self.dims['XDim'] = self.xdimsize
        if 'YDim' not in self.dims:
            self.dims['YDim'] = self.ydimsize

        _tuple = self._he.gdprojinfo(self.gridid)
        projcode, zonecode, spherecode, projparms = _tuple
        self.projcode = projcode
        self.zonecode = zonecode
        self.spherecode = spherecode
        self._sphere = _SPHERE[spherecode]
        self.projparms = projparms

        self.origincode = self._he.gdorigininfo(self.gridid)
        self.pixregcode = self._he.gdpixreginfo(self.gridid)

        if self.projcode == 22:
            self.offsets = self._he.gdblksomoffset(self.gridid)
            self.num_offsets = len(self.offsets) + 1

        # collect the fieldnames
        self._fields, _, _ = self._he.gdinqfields(self.gridid)
        self.fields = collections.OrderedDict()
        for fieldname in self._fields:
            self.fields[fieldname] = _GridVariable(self.gridid,
                                                   fieldname,
                                                   self._he)

        attr_list = self._he.gdinqattrs(self.gridid)
        self.attrs = collections.OrderedDict()
        for attr in attr_list:
            self.attrs[attr] = self._he.gdreadattr(self.gridid, attr)

    def __del__(self):
        self._he.gddetach(self.gridid)
        self._he.gdclose(self.gdfid)

    def _projection_str(self):
        """
        Return str representation of projection information.
        """
        if self.projcode == 0:
            title = "Projection:  Geographic"
            lst = []
        elif self.projcode == 1:
            title = "Projection:  UTM"
            lst = [self._projection_lonz_latz()]
        elif self.projcode == 3:
            title = "Projection:  Albers Conical Equal Area"
            lst = [self._projection_semi_major(),
                   self._projection_semi_minor(),
                   self._projection_latitudes_of_standard_parallels(),
                   self._projection_longitude_of_central_meridian(),
                   self._projection_latitude_of_projection_origin(),
                   self._projection_false_easting(),
                   self._projection_false_northing()]
        elif self.projcode == 6:
            title = "Projection:  Polar Stereographic"
            lst = [self._projection_semi_major(),
                   self._projection_semi_minor(),
                   self._projection_longitude_pole(),
                   self._projection_true_scale(),
                   self._projection_false_easting(),
                   self._projection_false_northing()]
        elif self.projcode == 11:
            title = "Projection:  Lambert Azimuthal"
            lst = [self._projection_sphere(),
                   self._projection_center_lon(),
                   self._projection_center_lat(),
                   self._projection_false_easting(),
                   self._projection_false_northing()]
        elif self.projcode == 16:
            title = "Projection:  Sinusoidal"
            lst = [self._projection_sphere(),
                   self._projection_longitude_of_central_meridian(),
                   self._projection_false_easting(),
                   self._projection_false_northing()]
        elif self.projcode == 22:
            if self.projparms[12] == 0:
                title = "Projection:  Space Oblique Mercator A"
                lst = [self._projection_incang(),
                       self._projection_asclong(),
                       self._projection_false_easting(),
                       self._projection_false_northing(),
                       self._projection_psrev(),
                       self._projection_srat(),
                       self._projection_pflag()]
            else:
                title = "Projection:  Space Oblique Mercator B"
                lst = [self._projection_semi_major(),
                       self._projection_semi_minor(),
                       self._projection_satnum(),
                       self._projection_path(),
                       self._projection_false_easting(),
                       self._projection_false_northing()]
        elif self.projcode == 97:
            title = "Projection:  CEA"
            lst = [self._projection_semi_major(),
                   self._projection_semi_minor(),
                   self._projection_longitude_of_central_meridian(),
                   self._projection_true_scale(),
                   self._projection_false_easting(),
                   self._projection_false_northing()]
        
        # Indent the projection title 4 spaces, indent the projection
        # parameters 8 spaces.
        if sys.hexversion < 0x03000000:
            msg = ' ' * 4 + title
            if len(lst) > 0:
                lst2 = [(' ' * 8 + line) for line in lst]
                msg += '\n' + '\n'.join(lst2)
        else:
            # Easy in python3
            msg = textwrap.indent(title, ' ' * 4)
            if len(lst) > 0:
                msg += ('\n' +
                        '\n'.join([textwrap.indent(x, ' ' * 8) for x in lst]))

        return msg

    def __str__(self):
        lst = ["Grid:  {0}".format(self.gridname)]
        lst.append("    Dimensions:")
        for dimname, dimlen in self.dims.items():
            lst.append("        {0}:  {1}".format(dimname, dimlen))

        lst.append("    Upper Left (x,y):  {0}".format(self.upleft))
        lst.append("    Lower Right (x,y):  {0}".format(self.lowright))
        lst.append("    Sphere:  {0}".format(self._sphere))

        lst.extend(self._projection_str().split('\n'))

        lst.append("    Fields:")
        for field in self.fields.keys():
            if sys.hexversion <= 0x03000000:
                textstr = str(self.fields[field])
                field_lst = [(' ' * 8 + line) for line in textstr.split('\n')]
                lst.extend(field_lst)
            else:
                lst.append(textwrap.indent(str(self.fields[field]), ' ' * 8))

        lst.append("    Grid Attributes:")
        for attr in self.attrs.keys():
            lst.append("        {0}:  {1}".format(attr, self.attrs[attr]))
        return '\n'.join(lst)

    def _projection_lonz_latz(self):
        """
        __str__ helper method for utm projections
        """
        if self.projparms[0] == 0 and self.projparms[1] == 0:
            msg = "UTM zone:  {0}".format(self.zonecode)
        else:
            lonz = self.projparms[0] / 1e6
            latz = self.projparms[1] / 1e6
            msg = "UTM zone longitude:  {0}\n".format(lonz)
            msg += "UTM zone latitude:  {0}".format(latz)
        return msg

    def _projection_pflag(self):
        """
        __str__ helper method for PFlag SOM parameter
        """
        item = self.projparms[10]
        msg = "End of path flag (0 = start, 1 = end):  {0}"
        return msg.format(item)

    def _projection_srat(self):
        """
        __str__ helper method for SRat SOM parameter
        """
        item = self.projparms[9]
        return "Satellite ratio start/end:  {0}".format(item)

    def _projection_asclong(self):
        """
        __str__ helper method for AscLong SOM parameter
        """
        item = self.projparms[4] / 1e6
        msg = "Longitude of ascending orbit at equator:  {:.6f}"
        return msg.format(item)

    def _projection_psrev(self):
        """
        __str__ helper method for PSRev SOM parameter
        """
        item = self.projparms[8]
        msg = "Period of satellite revolution:  {0} (min)"
        return msg.format(item)

    def _projection_incang(self):
        """
        __str__ helper method for IncAng SOM parameter
        """
        item = self.projparms[3] / 1e6
        msg = "Inclination of orbit at ascending node:  {:.6f}"
        return msg.format(item)

    def _projection_longitude_pole(self):
        """
        __str__ helper method for projections with longitude below pole of map
        """
        longpole = self.projparms[4] / 1e6
        return "Longitude below pole of map:  {0}".format(longpole)

    def _projection_true_scale(self):
        """
        __str__ helper method for projections with latitude of true scale
        """
        truescale = self.projparms[5] / 1e6
        return "Latitude of true scale:  {0}".format(truescale)

    def _projection_sphere(self):
        """
        __str__ helper method for projections with known ref sphere radius
        """
        sphere = self.projparms[0] / 1000
        if sphere == 0:
            sphere = 6370.997
        return "Radius of reference sphere(km):  {0}".format(sphere)

    def _projection_semi_major(self):
        """
        __str__ helper method for projections semi-major and semi-minor values
        """
        if self.projparms[0] == 0:
            # Clarke 1866
            semi_major = 6378.2064
        else:
            semi_major = self.projparms[0] / 1000
        msg = "Semi-major axis(km):  {0}".format(semi_major)
        return msg

    def _projection_semi_minor(self):
        """
        __str__ helper method for projections semi-major and semi-minor values
        """
        if self.projparms[0] == 0:
            # Clarke 1866
            semi_major = 6378.2064
        else:
            semi_major = self.projparms[0] / 1000
        if self.projparms[1] == 0:
            # spherical
            semi_minor = semi_major
        elif self.projparms[1] < 0:
            # eccentricity
            semi_minor = semi_major * np.sqrt(1 - self.projparms[1]**2)
        else:
            # semi minor axis
            semi_minor = self.projparms[1]
        msg = "Semi-minor axis(km):  {0}".format(semi_minor)
        return msg

    def _projection_latitudes_of_standard_parallels(self):
        """
        __str__ helper method for projections with 1st, 2nd standard parallels
        """
        msg = "Latitude of 1st Standard Parallel:  {0}\n"
        msg += "Latitude of 2nd Standard Parallel:  {1}"
        msg = msg.format(self.projparms[2]/1e6, self.projparms[3]/1e6)
        return msg

    def _projection_center_lon(self):
        """
        __str__ helper method for projections center of projection lat and lon
        """
        msg = "Center Longitude:  {0}".format(self.projparms[4]/1e6)
        return msg

    def _projection_center_lat(self):
        """
        __str__ helper method for projections center of projection lat and lon
        """
        msg += "Center Latitude:  {0}".format(self.projparms[5]/1e6)
        return msg

    def _projection_latitude_of_projection_origin(self):
        """
        __str__ helper method for latitude of projection origin
        """
        val = self.projparms[5]/1e6
        msg = "Latitude of Projection Origin:  {0}".format(val)
        return msg

    def _projection_longitude_of_central_meridian(self):
        """
        __str__ helper method for longitude of central meridian
        """
        val = self.projparms[4]/1e6
        msg = "Longitude of Central Meridian:  {0}".format(val)
        return msg

    def _projection_false_easting(self):
        """
        __str__ helper method for projections with false easting and northing
        """
        msg = "False Easting:  {0}".format(self.projparms[6])
        return msg

    def _projection_false_northing(self):
        """
        __str__ helper method for projections with false easting and northing
        """
        msg = "False Northing:  {0}".format(self.projparms[7])
        return msg

    def __getitem__(self, index):
        """
        Retrieve grid coordinates.
        """
        if self.projcode == 22:
            # The grid consists of the NBlocks, XDimSize, YDimSize
            shape = (self.dims['SOMBlockDim'],
                     self.dims['XDim'],
                     self.dims['YDim'])
        else:
            # The grid consists of the NBlocks, XDimSize, YDimSize
            shape = (self.dims['YDim'], self.dims['XDim'])

        if isinstance(index, int):
            raise RuntimeError("A scalar integer is not a legal argument.")

        if index is Ellipsis:
            # Case of [...]
            # Handle it below.
            if self.projcode == 22:
                # SOM projection, inherently 3D.
                bands = rows = cols = slice(None, None, None)
                return self.__getitem__((bands, rows, cols))
            else:
                # Other projections are 2D.
                rows = cols = slice(None, None, None)
                return self.__getitem__((rows, cols))

        if isinstance(index, slice):
            if (((index.start is None) and
                 (index.stop is None) and
                 (index.step is None))):
                # Case of grid[:]
                if self.projcode == 22:
                    # SOM projection, inherently 3D.
                    return self.__getitem__((index, index, index))
                else:
                    # Other projections are 2D.
                    return self.__getitem__((index, index))

            msg = "Single slice argument integer is only legal "
            msg += "if providing ':'"
            raise RuntimeError(msg)

        if isinstance(index, tuple) and len(index) > 2:
            if self.projcode != 22:
                msg = "More than two slice arguments are not allowed unless "
                msg += "the projection is SOM."
                raise RuntimeError(msg)

        if isinstance(index, tuple) and any(x is Ellipsis for x in index):
            # Remove the first ellipsis we find.
            if self.projcode == 22:
                # SOM projection is 3D
                rows = slice(0, self.xdimsize)
                cols = slice(0, self.ydimsize)
                bands = slice(0, self.dims['SOMBlockDim'])
                if index[0] is Ellipsis:
                    newindex = (bands, index[1], index[2])
                elif index[1] is Ellipsis:
                    newindex = (index[0], rows, index[2])
                else:
                    newindex = (index[0], index[1], cols)
            else:
                # Non-SOM projections are a lot easier.
                rows = slice(0, self.ydimsize)
                cols = slice(0, self.xdimsize)
                if index[0] is Ellipsis:
                    newindex = (rows, index[1])
                else:
                    newindex = (index[0], cols)

            # Easiest to just run it again.
            return self.__getitem__(newindex)

        if isinstance(index, tuple) and any(isinstance(x, int) for x in index):
            # Replace the first such integer argument, replace it with a slice.
            lst = list(index)
            predicate = lambda x: not isinstance(x[1], int)
            g = filterfalse(predicate, enumerate(index))
            idx = next(g)[0]
            lst[idx] = slice(index[idx], index[idx] + 1)
            newindex = tuple(lst)

            # Invoke array-based slicing again, as there may be additional
            # integer argument remaining.
            lat, lon = self.__getitem__(newindex)

            # Reduce dimensionality in the scalar dimension.
            lat = np.squeeze(lat, axis=idx)
            lon = np.squeeze(lon, axis=idx)
            return lat, lon

        # Assuming pargs is a tuple of slices from now on.
        # This is the workhorse section for the general case.
        if self.projcode == 22:
            # SOM grids are inherently 3D.  Must handle differently.
            return _som._get_som_grid(index, shape, self.offsets,
                                      self.upleft, self.lowright,
                                      self.projcode, self.projparms,
                                      self.spherecode)

        rows = index[0]
        cols = index[1]

        numrows = self.dims['YDim']
        numcols = self.dims['XDim']

        rows_start = 0 if rows.start is None else rows.start
        rows_step = 1 if rows.step is None else rows.step
        rows_stop = numrows if rows.stop is None else rows.stop
        cols_start = 0 if cols.start is None else cols.start
        cols_step = 1 if cols.step is None else cols.step
        cols_stop = numcols if cols.stop is None else cols.stop

        if (((rows_start < 0) or (rows_stop > numrows) or (cols_start < 0) or
             (cols_stop > numcols))):
            msg = "Grid index row/col arguments are out of bounds."
            raise RuntimeError(msg)

        col = np.arange(cols_start, cols_stop, cols_step)
        row = np.arange(rows_start, rows_stop, rows_step)
        cols, rows = np.meshgrid(col, row)
        cols = cols.astype(np.int32)
        rows = rows.astype(np.int32)
        lon, lat = self._he.gdij2ll(self.projcode,
                                    self.zonecode, self.projparms,
                                    self.spherecode,
                                    self.xdimsize, self.ydimsize,
                                    self.upleft, self.lowright,
                                    rows, cols,
                                    self.pixregcode, self.origincode)
        return lat, lon


class GridFile(object):
    """
    Access to HDF-EOS grid files.

    Attributes
    ----------
    filename : str
        HDF-EOS2 or HDF-EOS5 grid file
    grids : dictionary
        collection of grids
    """
    def __init__(self, filename):
        self.filename = filename
        try:
            self.gdfid = he4.gdopen(filename)
            self._he = he4
        except IOError:
            # try hdf5
            self.gdfid = he5.gdopen(filename)
            self._he = he5

        gridlist = self._he.gdinqgrid(filename)
        self.grids = collections.OrderedDict()
        for gridname in gridlist:
            self.grids[gridname] = _Grid(self.filename, gridname, self._he)
            if not hasattr(self._he, 'gdinqlocattrs'):
                # Inquire about hdf4 attributes using SD interface
                for fieldname in self.grids[gridname].fields.keys():
                    attrs = self._hdf4_attrs(filename, gridname, fieldname)
                    self.grids[gridname].fields[fieldname].attrs = attrs

    def _hdf4_attrs(self, filename, gridname, fieldname):
        """
        Retrieve field attributes using HDF4 interface.
        """

        attrs = None

        fid = hdf.hopen(filename)
        sd_id = hdf.sdstart(filename)
        hdf.vstart(fid)

        grid_ref = hdf.vfind(fid, gridname)
        grid_vg = hdf.vattach(fid, grid_ref)

        members = hdf.vgettagrefs(grid_vg)
        for tag_i, ref_i in members:
            if tag_i == hdf.DFTAG_VG:
                # Descend into a vgroup if we find it.
                vg0 = hdf.vattach(fid, ref_i)
                name = hdf.vgetname(vg0)
                if name == 'Data Fields':
                    # We want this Vgroup
                    df_members = hdf.vgettagrefs(vg0)
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
                hdf.vdetach(vg0)

        hdf.vdetach(grid_vg)

        hdf.vend(fid)
        hdf.sdend(sd_id)
        hdf.hclose(fid)

        if attrs is None:
            # No attributes.
            attrs = collections.OrderedDict()
        return attrs

    def __repr__(self):
        return "GridFile('{0}')".format(self.filename)

    def __str__(self):
        orig_printoptions = np.get_printoptions()
        np.set_printoptions(precision=6)
        lst = ["{0}".format(os.path.basename(self.filename))]
        for grid in self.grids.keys():
            lst.append(str(self.grids[grid]))
        np.set_printoptions(**orig_printoptions)
        return '\n'.join(lst)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        pass

    def __del__(self):
        self._he.gdclose(self.gdfid)


_SPHERE = {-1: 'Unspecified',
           0: 'Clarke 1866',
           1: 'Clarke 1880',
           2: 'Bessel',
           3: 'International 1967',
           4: 'International 1909',
           5: 'WGS 72',
           6: 'Everest',
           7: 'WGS 66',
           8: 'GRS 1980',
           9: 'Airy',
           10: 'Modified Airy',
           11: 'Modified Everest',
           12: 'WGS 84',
           13: 'Southeast Asia',
           14: 'Australian National',
           15: 'Krassovsky',
           16: 'Hough',
           17: 'Mercury 1960',
           18: 'Modified Mercury 1968',
           19: 'Sphere of Radius 6370997m',
           20: 'Sphere of Radius 6371228m',
           21: 'Sphere of Radius 6371007.181'}
