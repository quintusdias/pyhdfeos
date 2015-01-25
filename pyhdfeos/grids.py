"""
Support for HDF-EOS and HDF-EOS5 grid files
"""

import collections
import os
import sys

if sys.hexversion < 0x03000000:
    from itertools import ifilterfalse as filterfalse
else:
    from itertools import filterfalse

import numpy as np

from .lib import he4, he5, hdf
from .core import EosFile, _EosStruct, _EosField
from . import _som


class _GridVariable(_EosField):
    """
    Grid field object (data, dimensions, attributes)

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
    def __init__(self, gridid, fieldname, he_module):
        _EosField.__init__(self)
        self._id = gridid
        self.fieldname = fieldname
        self._he = he_module

        x = self._he.gdfieldinfo(self._id, fieldname)
        self.shape, self.dtype, self.dims = x[0:3]

        # HDFEOS5 only.
        self.attrs = collections.OrderedDict()
        if hasattr(self._he, 'gdinqlocattrs'):
            attr_names = self._he.gdinqlocattrs(self._id, self.fieldname)
            for attrname in attr_names:
                self.attrs[attrname] = self._he.gdreadlocattr(self._id,
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
        return self._he.gdreadfield(self._id, self.fieldname,
                                    start, stride, edge)


class _Grid(_EosStruct):
    """
    Grid object, concerned only with coordinates of HDF-EOS grids.

    Attributes
    ----------
    name : str
       name of grid
    fields : dictionary
        collection of data field objects defined in the grid
    dims : dict
       list of dimension names and extents defined in the grid
    attrs : dict
        collection of swath attributes
    xdimsize, ydimsize : int
        number of columns and rows defining the grid
    upleft, lowright : 2-element numpy ndarray
        upper left and lower right coordinate points of grid
    projcode : int
        GCTP projection code
    zonecode : int
        UTM zone code
    spherecode : int
        identifies the GCTP spheroid
    projparms : array
        GCTP projection parameters
    origincode : int
        identifies the origin of the pixels in grid data
    pixregcode : int
        identifies the pixel registration

    For complete details concerning projcode, zonecode, spherecode, projparms,
    origincode, pixregcode, please consult "The HDF-EOS Library Users Guide
    for the EOSDIS Evolution and Development (EED) Contract"
    """
    def __init__(self, filename, gridname, he_module):
        self._filename = filename
        self._he = he_module
        self._gdfid = self._he.gdopen(filename)
        self._gridid = self._he.gdattach(self._gdfid, gridname)
        self.name = gridname

        dimnames, dimlens = self._he.gdinqdims(self._gridid)
        dims = [(k, v) for (k, v) in zip(dimnames, dimlens)]
        self.dims = collections.OrderedDict(dims)

        _tuple = self._he.gdgridinfo(self._gridid)
        self.xdimsize, self.ydimsize, self.upleft, self.lowright = _tuple
        if hasattr(self._he, 'gdinqlocattrs'):
            # HDF-EOS5
            if 'Xdim' not in self.dims:
                self.dims['Xdim'] = self.xdimsize
            if 'Ydim' not in self.dims:
                self.dims['Ydim'] = self.ydimsize
        else:
            # HDF-EOS
            if 'XDim' not in self.dims:
                self.dims['XDim'] = self.xdimsize
            if 'YDim' not in self.dims:
                self.dims['YDim'] = self.ydimsize

        _tuple = self._he.gdprojinfo(self._gridid)
        projcode, zonecode, spherecode, projparms = _tuple
        self.projcode = projcode
        self.zonecode = zonecode
        self.spherecode = spherecode
        self._sphere = _SPHERE[spherecode]
        self.projparms = projparms

        self.origincode = self._he.gdorigininfo(self._gridid)
        self.pixregcode = self._he.gdpixreginfo(self._gridid)

        if self.projcode == 22:
            self.offsets = self._he.gdblksomoffset(self._gridid)
            self.num_offsets = len(self.offsets) + 1

        # collect the fieldnames
        self._fields, _, _ = self._he.gdinqfields(self._gridid)
        self.fields = collections.OrderedDict()
        for fieldname in self._fields:
            self.fields[fieldname] = _GridVariable(self._gridid,
                                                   fieldname,
                                                   self._he)

        attr_list = self._he.gdinqattrs(self._gridid)
        self.attrs = collections.OrderedDict()
        for attr in attr_list:
            self.attrs[attr] = self._he.gdreadattr(self._gridid, attr)

    def __del__(self):
        self._he.gddetach(self._gridid)
        self._he.gdclose(self._gdfid)

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

        if len(lst) == 0:
            return title

        text = '\n'.join(lst)
        text = self._textwrap(text, 4)

        text = title + '\n' + text

        return text

    def __str__(self):
        title = "Grid:  {0}".format(self.name)

        lst = []

        text = self._format_dimensions()
        lst.append(text)

        lst.append("Upper Left (x,y):  {0}".format(self.upleft))
        lst.append("Lower Right (x,y):  {0}".format(self.lowright))
        lst.append("Sphere:  {0}".format(self._sphere))

        text = self._projection_str()
        lst.append(text)

        text = self._format_fields('Fields', self.fields)
        lst.append(text)

        text = self._format_attributes('Grid Attributes', self.attrs)
        lst.append(text)

        text = '\n'.join(lst)
        text = self._textwrap(text, 4)

        text = title + '\n' + text

        return text

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
        msg = "Center Latitude:  {0}".format(self.projparms[5]/1e6)
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
            # The grid consists of the YDimSize, XDimSize
            shape = (self.ydimsize, self.xdimsize)

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
            # Usually this case means that the user invoked something like
            #     data = obj.grid[:]
            # i.e. wants the entire grid.
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

            # If we are still here, then the user supplied a single custom
            # slice argument, something like
            #     rows = slice(2,8,2)
            #     data = obj.grid[rows]
            # We don't want to have to deal with that.
            msg = "A Single slice argument is only legal if providing ':'"
            raise RuntimeError(msg)

        if isinstance(index, tuple) and len(index) > 2:
            if self.projcode != 22:
                msg = "More than two slice arguments are not allowed unless "
                msg += "the projection is SOM."
                raise RuntimeError(msg)

        if isinstance(index, tuple) and any(x is Ellipsis for x in index):
            # Remove the first ellipsis we find, then just rerun.
            newindex = self._remove_first_ellipsis_from_index(index)
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

        numrows = self.ydimsize
        numcols = self.xdimsize

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

    def _remove_first_ellipsis_from_index(self, index):
        """
        resolve first ellipsis into appropriate values

        Parameters
        ----------
        index : tuple
            the index passed into the __getitem__ method is a tuple with at
            least one ellipsis
        """
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

        return newindex


class GridFile(EosFile):
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
        EosFile.__init__(self, filename)
        try:
            self._gdfid = he4.gdopen(filename)
            self._he = he4
        except IOError:
            # try hdf5
            try:
                self._gdfid = he5.gdopen(filename)
            except IOError:
                msg = "Unable to open {0} as either an HDF-EOS or HDF-EOS5 "
                msg += "grid file."
                raise RuntimeError(msg.format(filename))
            self._he = he5

        gridlist = self._he.gdinqgrid(filename)
        self.grids = collections.OrderedDict()
        for gridname in gridlist:
            self.grids[gridname] = _Grid(self.filename, gridname, self._he)
            if self._is_hdf4:
                self._position_to_hdf4_vgroup(gridname)
                # Inquire about hdf4 attributes using SD interface
                for fieldname in self.grids[gridname].fields.keys():
                    attrs = self._get_sds_attributes(fieldname)
                    self.grids[gridname].fields[fieldname].attrs = attrs

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
        if self._he is not None:
            self._he.gdclose(self._gdfid)

        # Close the HDF4 raw file IDs
        if self._is_hdf4:
            hdf.vend(self._fid)
            hdf.sdend(self._sd_id)
            hdf.hclose(self._fid)


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
