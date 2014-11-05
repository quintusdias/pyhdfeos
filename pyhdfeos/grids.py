import os
import platform
import struct

import numpy as np

from . import core
from .lib import he4, he5

class _Grid(object):
    """
    """
    def __init__(self, gdfid, gridname, he_module):
        self._he = he_module
        self.gridid = self._he.gdattach(gdfid, gridname)
        self.gridname = gridname

        self.shape, self.upleft, self.lowright = self._he.gdgridinfo(self.gridid)

        projcode, zonecode, spherecode, projparms = self._he.gdprojinfo(self.gridid)
        self.projcode = projcode
        self.zonecode = zonecode
        self.spherecode = spherecode
        self.projparms = projparms

        self.origincode = self._he.gdorigininfo(self.gridid)
        self.pixregcode = self._he.gdpixreginfo(self.gridid)

        self._fields, _, _ = self._he.gdinqfields(self.gridid)

        attr_list = self._he.gdinqattrs(self.gridid)
        self.attrs = {}
        for attr in attr_list:
            self.attrs[attr] = self._he.gdreadattr(self.gridid, attr)

    def __del__(self):
        self._he.gddetach(self.gridid)

    def __str__(self):
        msg = "Grid:  {0}\n".format(self.gridname)
        msg += "    Shape:  {0}\n".format(self.shape)
        msg += "    Upper Left (x,y):  {0}\n".format(self.upleft)
        msg += "    Lower Right (x,y):  {0}\n".format(self.lowright)
        if self.projcode == 0:
            msg += "    Projection:  Geographic\n"
        elif self.projcode == 3:
            msg += "    Projection:  Albers Conical Equal Area\n"
            msg += self._projection_semi_major_semi_minor()
            msg += self._projection_latitudes_of_standard_parallels()
            msg += self._projection_longitude_of_central_meridian()
            msg += self._projection_latitude_of_projection_origin()
            msg += self._projection_false_easting_northing()
        elif self.projcode == 11:
            msg += "    Projection:  Lambert Azimuthal\n"
            msg += self._projection_sphere()
            msg += self._projection_center_lon_lat()
            msg += self._projection_false_easting_northing()
        elif self.projcode == 16:
            msg += "    Projection:  Sinusoidal\n"
            msg += self._projection_sphere()
            msg += self._projection_longitude_of_central_meridian()
            msg += self._projection_false_easting_northing()

        msg += "    Fields:\n"
        for field in self._fields:
            msg += "        {0}:\n".format(field)

        msg += "    Attributes:\n"
        for attr in self.attrs.keys():
            msg += "        {0}:  {1}\n".format(attr, self.attrs[attr])

        
        return msg

    def _projection_sphere(self):
        """
        __str__ helper method for projections with known reference sphere radius
        """
        sphere = self.projparms[0] / 1000
        if sphere == 0:
            sphere = 6370.997
        return "        Radius of reference sphere(km):  {0}\n".format(sphere)

    def _projection_semi_major_semi_minor(self):
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
        msg = "        Semi-major axis(km):  {0}\n".format(semi_major)
        msg += "        Semi-minor axis(km):  {0}\n".format(semi_minor)
        return msg

    def _projection_latitudes_of_standard_parallels(self):
        """
        __str__ helper method for projections with 1st, 2nd standard parallels
        """
        msg = "        Latitude of 1st Standard Parallel:  {0}\n"
        msg += "        Latitude of 2nd Standard Parallel:  {1}\n"
        msg = msg.format(self.projparms[2]/1e6, self.projparms[3]/1e6)
        return msg

    def _projection_center_lon_lat(self):
        """
        __str__ helper method for projections center of projection lat and lon
        """
        msg = "        Center Longitude:  {0}\n".format(self.projparms[4]/1e6)
        msg += "        Center Latitude:  {0}\n".format(self.projparms[5]/1e6)
        return msg

    def _projection_latitude_of_projection_origin(self):
        """
        __str__ helper method for latitude of projection origin
        """
        val = self.projparms[5]/1e6
        msg = "        Latitude of Projection Origin:  {0}\n".format(val)
        return msg

    def _projection_longitude_of_central_meridian(self):
        """
        __str__ helper method for longitude of central meridian
        """
        val = self.projparms[4]/1e6
        msg = "        Longitude of Central Meridian:  {0}\n".format(val)
        return msg

    def _projection_false_easting_northing(self):
        """
        __str__ helper method for projections with false easting and northing
        """
        msg = "        False Easting:  {0}\n".format(self.projparms[6])
        msg += "        False Northing:  {0}\n".format(self.projparms[7])
        return msg

    def __getitem__(self, index):
        """
        Retrieve grid coordinates.
        """
        numrows, numcols = self.shape

        if isinstance(index, int):
            raise RuntimeError("A scalar integer is not a legal argument.")

        if index is Ellipsis:
            # Case of [...]
            # Handle it below.
            rows = cols = slice(None, None, None)
            return self.__getitem__((rows, cols))

        if isinstance(index, slice):
            if index.start is None and index.stop is None and index.step is None:
                # Case of jp2[:]
                return self.__getitem__((index,index))

            msg = "Single slice argument integer is only legal if ':'"
            raise RuntimeError(msg)

        if isinstance(index, tuple) and len(index) > 2:
            msg = "More than two slice arguments are not allowed."
            raise RuntimeError(msg)

        if isinstance(index, tuple) and any(x is Ellipsis for x in index):
            # Remove the first ellipsis we find.
            rows = slice(0, numrows)
            cols = slice(0, numcols)
            if index[0] is Ellipsis:
                newindex = (rows, index[1])
            else:
                newindex = (index[0], cols)

            # Run once again because it is possible that there's another
            # Ellipsis object.
            return self.__getitem__(newindex)

        if isinstance(index, tuple) and any(isinstance(x, int) for x in index):
            # Replace the first such integer argument, replace it with a slice.
            lst = list(pargs)
            predicate = lambda x: not isinstance(x[1], int)
            g = filterfalse(predicate, enumerate(pargs))
            idx = next(g)[0]
            lst[idx] = slice(pargs[idx], pargs[idx] + 1)
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
        rows = index[0]
        cols = index[1]

        rows_start = 0 if rows.start is None else rows.start
        rows_step = 1 if rows.step is None else rows.step
        rows_stop = numrows if rows.stop is None else rows.stop
        cols_start = 0 if cols.start is None else cols.start
        cols_step = 1 if cols.step is None else cols.step
        cols_stop = numcols if cols.stop is None else cols.stop

        if (((rows_start < 0) or (rows_stop > numrows) or (cols_start < 0) or
             (cols_stop > numcols))):
            msg = "Grid index arguments are out of bounds."
            raise RuntimeError(msg)

        col = np.arange(cols_start, cols_stop, cols_step)
        row = np.arange(rows_start, rows_stop, rows_step)
        cols, rows = np.meshgrid(col,row)
        cols = cols.astype(np.int32)
        rows = rows.astype(np.int32)
        lon, lat = self._he.gdij2ll(self.projcode, self.zonecode, self.projparms,
                             self.spherecode, self.shape[1], self.shape[0],
                             self.upleft, self.lowright,
                             rows, cols,
                             self.pixregcode, self.origincode)
        return lat, lon


class GridFile(object):
    """
    Access to HDF-EOS grid files.
    """
    def __init__(self, filename):
        self.filename = filename
        try:
            self.gdfid = he4.gdopen(filename)
            self._he = he4
        except IOError as err:
            self.gdfid = he5.gdopen(filename)
            self._he = he5

        gridlist = self._he.gdinqgrid(filename)
        self.grids = {}
        for gridname in gridlist:
            self.grids[gridname] = _Grid(self.gdfid, gridname, self._he)

    def __str__(self):
        msg = "{0}\n".format(os.path.basename(self.filename))
        for grid in self.grids.keys():
            msg += str(self.grids[grid])
        return msg

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        pass

    def __del__(self):
        """
        Clean up any open grids, close the file
        """
        for gridname in self.grids:
            grid = self.grids[gridname]
            self.grids[gridname] = None
            del grid
        self._he.gdclose(self.gdfid)


