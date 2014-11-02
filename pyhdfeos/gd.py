from contextlib import contextmanager
import pkg_resources
import platform

import numpy as np
from cffi import FFI

from . import core

ffi = FFI()
ffi.cdef("""
        typedef int int32;
        typedef int intn;
        typedef double float64;

        int32 GDattach(int32 gdfid, char *grid);
        intn  GDdetach(int32 gid);
        intn  GDclose(int32 fid);
        int32 GDij2ll(int32 projcode, int32 zonecode,
                      float64 projparm[], int32 spherecode, int32 xdimsize,
                      int32 ydimsize, float64 upleft[], float64 lowright[],
                      int32 npts, int32 row[], int32 col[], float64
                      longititude[], float64 latitude[], int32 pixcen,
                      int32 pixcnr);
        int32 GDinqfields(int32 gridid, char *fieldlist, int32 rank[],
                          int32 numbertype[]);
        int32 GDinqgrid(char *filename, char *gridlist, int32 *strbufsize);
        int32 GDnentries(int32 gridid, int32 entrycode, int32 *strbufsize);
        intn  GDgridinfo(int32 gridid, int32 *xdimsize, int32 *ydimsize,
                         float64 upleft[2], float64 lowright[2]);
        int32 GDopen(char *name, intn access);
        intn  GDorigininfo(int32 gridid, int32 *origincode);
        intn  GDpixreginfo(int32 gridid, int32 *pixregcode);
        intn  GDprojinfo(int32 gridid, int32 *projcode, int32 *zonecode,
                         int32 *spherecode, float64 projparm[]);
        """)

if platform.system().startswith('Linux'):
    if platform.linux_distribution() == ('Fedora', '20', 'Heisenbug'):
        libraries=['hdfeos', 'Gctp', 'mfhdf', 'df', 'jpeg', 'z']
    else:
        libraries=['hdfeos', 'gctp', 'mfhdf', 'df', 'jpeg', 'z']
else:
    libraries=['hdfeos', 'Gctp', 'mfhdf', 'df', 'jpeg', 'z']
_lib = ffi.verify("""
        #include "mfhdf.h"
        #include "HE2_config.h"
        #include "HdfEosDef.h"
        """,
        libraries=libraries,
        include_dirs=['/usr/include/hdf',
                      '/usr/include/x86_64-linux-gnu/hdf',
                      '/opt/local/include',
                      '/usr/local/include'],
        library_dirs=['/usr/lib/hdf', '/opt/local/lib', '/usr/local/lib'])

class Grid(object):
    """
    """
    def __init__(self, gdfid, gridname):
        self.gridid = attach(gdfid, gridname)
        self.gridname = gridname

        (numrows, numcols), upleft, lowright = gridinfo(self.gridid)
        self.shape = (numrows, numcols)
        self.upleft = upleft
        self.lowright = lowright

        projcode, zonecode, spherecode, projparms = projinfo(self.gridid)
        self.projcode = projcode
        self.zonecode = zonecode
        self.spherecode = spherecode
        self.projparms = projparms

        self.origincode = origininfo(self.gridid)
        self.pixregcode = pixreginfo(self.gridid)

        self._fields, _, _ = inqfields(self.gridid)

    def __del__(self):
        detach(self.gridid)

    def __str__(self):
        msg = "Grid:  {0}\n".format(self.gridname)
        msg += "Shape:  {0}\n".format(self.shape)
        msg += "Upper Left (x,y):  {0}\n".format(self.upleft)
        msg += "Lower Right (x,y):  {0}\n".format(self.lowright)
        if self.projcode == 0:
            msg += "Projection:  Geographic\n"
        elif self.projcode == 3:
            msg += "Projection:  Albers Conical Equal Area\n"
            msg += self._projection_semi_major_semi_minor()
            msg += self._projection_latitudes_of_standard_parallels()
            msg += self._projection_longitude_of_central_meridian()
            msg += self._projection_latitude_of_projection_origin()
            msg += self._projection_false_easting_northing()
        elif self.projcode == 11:
            msg += "Projection:  Lambert Azimuthal\n"
            msg += self._projection_sphere()
            msg += self._projection_center_lon_lat()
            msg += self._projection_false_easting_northing()
        elif self.projcode == 16:
            msg += "Projection:  Sinusoidal\n"
            msg += self._projection_sphere()
            msg += self._projection_longitude_of_central_meridian()
            msg += self._projection_false_easting_northing()

        msg += "Fields:\n"
        for field in self._fields:
            msg += "    {0}:\n".format(field)
        return msg

    def _projection_sphere(self):
        """
        __str__ helper method for projections with known reference sphere radius
        """
        sphere = self.projparms[0] / 1000
        if sphere == 0:
            sphere = 6370.997
        return "    Radius of reference sphere(km):  {0}\n".format(sphere)

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
        msg = "    Semi-major axis(km):  {0}\n".format(semi_major)
        msg += "    Semi-minor axis(km):  {0}\n".format(semi_minor)
        return msg

    def _projection_latitudes_of_standard_parallels(self):
        """
        __str__ helper method for projections with 1st, 2nd standard parallels
        """
        msg = "    Latitude of 1st Standard Parallel:  {0}\n"
        msg += "    Latitude of 2nd Standard Parallel:  {1}\n"
        msg = msg.format(self.projparms[2]/1e6, self.projparms[3]/1e6)
        return msg

    def _projection_center_lon_lat(self):
        """
        __str__ helper method for projections center of projection lat and lon
        """
        msg = "    Center Longitude:  {0}\n".format(self.projparms[4]/1e6)
        msg += "    Center Latitude:  {0}\n".format(self.projparms[5]/1e6)
        return msg

    def _projection_latitude_of_projection_origin(self):
        """
        __str__ helper method for latitude of projection origin
        """
        val = self.projparms[5]/1e6
        msg = "    Latitude of Projection Origin:  {0}\n".format(val)
        return msg

    def _projection_longitude_of_central_meridian(self):
        """
        __str__ helper method for longitude of central meridian
        """
        val = self.projparms[4]/1e6
        msg = "    Longitude of Central Meridian:  {0}\n".format(val)
        return msg

    def _projection_false_easting_northing(self):
        """
        __str__ helper method for projections with false easting and northing
        """
        msg = "    False Easting:  {0}\n".format(self.projparms[6])
        msg += "    False Northing:  {0}\n".format(self.projparms[7])
        return msg

    def __getitem__(self, index):
        """
        Retrieve grid coordinates.
        """
        (numrows, numcols), _, _ = gridinfo(self.gridid)

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
        lon, lat = ij2ll(self.projcode, self.zonecode, self.projparms,
                         self.spherecode, self.shape[1], self.shape[0],
                         self.upleft, self.lowright,
                         rows, cols,
                         self.pixregcode, self.origincode)
        return lat, lon


class GridFile(object):
    """
    Access to HDF-EOS grid files.
    """
    def __init__(self, filename, access=core.DFACC_READ):
        self.filename = filename,
        self.access = access
        self.gdfid = open(filename, access=access)

        gridlist = inqgrid(filename)
        self.grids = {}
        for gridname in gridlist:
            self.grids[gridname] = Grid(self.gdfid, gridname)

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
        close(self.gdfid)


def _handle_error(status):
    if status < 0:
        raise IOError("Library routine failed.")

def get_grid(filename, gridname=None):
    """
    """
    if gridname is None:
        gridlist = inqgrid(filename)
        gridname = gridlist[0]
    with open(filename) as gdfid:
        with attach(gdfid, gridname) as gridid:
            projcode, zonecode, spherecode, projparms = projinfo(gridid)
            (nrow, ncol), upleft, lowright = gridinfo(gridid)
            pixcen = pixreginfo(gridid)
            pixcnr = origininfo(gridid)
            col = np.arange(ncol)
            col = col.astype(np.int32)
            lon = np.zeros((nrow, ncol))
            lat = np.zeros((nrow, ncol))
            for j in range(nrow):
                row = j * np.ones((ncol,), dtype = np.int32)
                _lon, _lat = ij2ll(projcode, zonecode, projparms, spherecode,
                                   ncol, nrow, upleft, lowright, row, col,
                                   pixcen, pixcnr)
                lat[j,:] = _lat
                lon[j,:] = _lon
            return lon, lat


def attach(gdfid, gridname):
    """Attach to an existing grid structure.

    Parameters
    ----------
    gdfid : int
        Grid file id.
    gridname : str
        Name of grid to be attached.

    Returns
    -------
    grid_id : int
        Grid identifier.

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    return _lib.GDattach(gdfid, gridname.encode())

def close(gdfid):
    """Close an HDF-EOS file.

    Parameters
    ----------
    fid : int
        Grid file id.

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    status = _lib.GDclose(gdfid)
    _handle_error(status)

def detach(grid_id):
    """Detach from grid structure.

    Parameters
    ----------
    grid_id : int
        Grid identifier.

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    status = _lib.GDdetach(grid_id)
    _handle_error(status)

def gridinfo(grid_id):
    """Return information about a grid structure.

    Parameters
    ----------
    grid_id : int
        Grid identifier.

    Returns
    -------
    gridsize : tuple
        Number of rows, columns in grid.
    upleft, lowright : np.float64[2]
        Location in meters of upper left, lower right corners.

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    xdimsize = ffi.new("int32 *")
    ydimsize = ffi.new("int32 *")
    upleft_buffer = ffi.new("float64[]", 2)
    lowright_buffer = ffi.new("float64[]", 2)
    status = _lib.GDgridinfo(grid_id, xdimsize, ydimsize,
                             upleft_buffer, lowright_buffer)
    _handle_error(status)

    gridsize = (ydimsize[0], xdimsize[0])

    upleft = np.zeros(2, dtype=np.float64)
    upleft[0] = upleft_buffer[0]
    upleft[1] = upleft_buffer[1]

    lowright = np.zeros(2, dtype=np.float64)
    lowright[0] = lowright_buffer[0]
    lowright[1] = lowright_buffer[1]

    return gridsize, upleft, lowright

def ij2ll(projcode, zonecode, projparm, spherecode, xdimsize, ydimsize, upleft,
          lowright, row, col, pixcen, pixcnr):
    """Convert coordinates (i, j) to (longitude, latitude).

    Parameters
    ----------
    projcode : int
        GCTP projection code
    zonecode : int
        GCTP zone code used by UTM projection
    projparm : ndarray
        Projection parameters.
    spherecode : int
        GCTP spherecode
    xdimsize, ydimsize : int
        Size of grid.
    upleft, lowright : ndarray
        Upper left, lower right corner of the grid in meter (all projections
        except Geographic) or DMS degree (Geographic).
    row, col : ndarray
        row, column numbers of the pixels (zero based)

    Returns
    -------
    longitude, latitude : ndarray
        Longitude and latitude in decimal degrees.

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    longitude = np.zeros(col.shape, dtype=np.float64)
    latitude = np.zeros(col.shape, dtype=np.float64)
    upleftp = ffi.cast("float64 *", upleft.ctypes.data)
    lowrightp = ffi.cast("float64 *", lowright.ctypes.data)
    projparmp = ffi.cast("float64 *", projparm.ctypes.data)
    colp = ffi.cast("int32 *", col.ctypes.data)
    rowp = ffi.cast("int32 *", row.ctypes.data)
    longitudep = ffi.cast("float64 *", longitude.ctypes.data)
    latitudep = ffi.cast("float64 *", latitude.ctypes.data)
    status = _lib.GDij2ll(projcode, zonecode, projparmp, spherecode,
                          xdimsize, ydimsize, upleftp, lowrightp, col.size,
                          rowp, colp, longitudep, latitudep, pixcen, pixcnr)
    return longitude, latitude

def inqfields(gridid):
    """Retrieve information about data fields defined in a grid.

    Returns
    -------
    fields : list
        List of fields in the grid.
    ranks : list
        List of ranks corresponding to the fields
    numbertypes : list
        List of numbertypes corresponding to the fields

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    nfields, strbufsize = nentries(gridid, core.HDFE_NENTFLD)
    fieldlist_buffer = ffi.new("char[]", b'\0' * (strbufsize + 1))
    rank_buffer = ffi.new("int[]", nfields)
    numbertype_buffer = ffi.new("int[]", nfields)
    nfields2 = _lib.GDinqfields(gridid, fieldlist_buffer,
                                rank_buffer, numbertype_buffer)
    fieldlist = ffi.string(fieldlist_buffer).decode('ascii').split(',')

    ranks = []
    numbertypes = []
    for j in range(len(fieldlist)):
        ranks.append(rank_buffer[j])
        numbertypes.append(numbertype_buffer[j])

    return fieldlist, ranks, numbertypes

def inqgrid(filename):
    """Retrieve grid structures defined in HDF-EOS file.

    Parameters
    ----------
    grid_id : int
        Grid identifier.

    Returns
    -------
    gridlist : list
        List of grids defined in HDF-EOS file.

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    strbufsize = ffi.new("int32 *")
    ngrids = _lib.GDinqgrid(filename.encode(), ffi.NULL, strbufsize)
    gridbuffer = ffi.new("char[]", b'\0' * (strbufsize[0] + 1))
    ngrids = _lib.GDinqgrid(filename.encode(), gridbuffer, ffi.NULL)
    _handle_error(ngrids)
    gridlist = ffi.string(gridbuffer).decode('ascii').split(',')
    return gridlist

def nentries(gridid, entry_code):
    """Return number of specified objects in a grid.

    Parameters
    ----------
    grid_id : int
        Grid identifier.
    entry_code : int
        Entry code, either HDFE_NENTDIM or HDFE_NENTDFLD

    Returns
    -------
    nentries, strbufsize : tuple of ints
       Number of specified entries, number of bytes in descriptive strings. 

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    strbufsize = ffi.new("int32 *")
    nentries = _lib.GDnentries(gridid, entry_code, strbufsize)
    return nentries, strbufsize[0]

def open(filename, access=core.DFACC_READ):
    return _lib.GDopen(filename.encode(), access)

def origininfo(grid_id):
    """Return grid pixel origin information.

    Parameters
    ----------
    grid_id : int
        Grid identifier.

    Returns
    -------
    origincode : int
        Origin code.

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    origincode = ffi.new("int32 *")
    status = _lib.GDorigininfo(grid_id, origincode)
    _handle_error(status)

    return origincode[0]

def pixreginfo(grid_id):
    """Return pixel registration information.

    Parameters
    ----------
    grid_id : int
        Grid identifier.

    Returns
    -------
    pixregcode : int
        Pixel registration code.

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    pixregcode = ffi.new("int32 *")
    status = _lib.GDpixreginfo(grid_id, pixregcode)
    _handle_error(status)

    return pixregcode[0]

def projinfo(grid_id):
    """Return grid projection information.

    Parameters
    ----------
    grid_id : int
        Grid identifier.

    Returns
    -------
    projode : int
        GCTP projection code.
    zonecode : int
        GCTP zone code used by UTM projection.
    spherecode : int
        GCTP spheroid code
    projparm : ndarray
        GCTP projection parameters

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    projcode = ffi.new("int32 *")
    zonecode = ffi.new("int32 *")
    spherecode = ffi.new("int32 *")
    projparm = np.zeros(13, dtype=np.float64)
    projparmp = ffi.cast("float64 *", projparm.ctypes.data)
    status = _lib.GDprojinfo(grid_id, projcode, zonecode, spherecode,
                             projparmp)
    _handle_error(status)

    return projcode[0], zonecode[0], spherecode[0], projparm

