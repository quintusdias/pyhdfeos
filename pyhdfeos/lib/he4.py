import os
import platform
import sys

import numpy as np
from cffi import FFI

from . import config 

CDEF = """
    typedef float float32;
    typedef int int32;
    typedef int intn;
    typedef double float64;

    intn  EHidinfo(int32 fid, int32 *hdfid, int32 *sdid);
    int32 GDattach(int32 gdfid, char *grid);
    intn  GDattrinfo(int32 gdfid, char *attrname, int32 *nbyte, int32
                     *count);
    intn  GDblkSOMoffset(int32 fid, float32 [], int32 count, char *code);
    intn  GDdetach(int32 gid);
    intn  GDclose(int32 fid);
    intn  GDfieldinfo(int32 gridid, char *fieldname, int32 *rank,
                      int32 dims[], int32 *numbertype, char *dimlist);
    int32 GDij2ll(int32 projcode, int32 zonecode,
                  float64 projparm[], int32 spherecode, int32 xdimsize,
                  int32 ydimsize, float64 upleft[], float64 lowright[],
                  int32 npts, int32 row[], int32 col[], float64
                  longititude[], float64 latitude[], int32 pixcen,
                  int32 pixcnr);
    int32 GDinqattrs(int32 gridid, char *attrlist, int32 *strbufsize);
    int32 GDinqdims(int32 gridid, char *dimname, int32 *dims);
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
    intn  GDreadattr(int32 gridid, char* attrname, void *buffer);
    intn  GDreadfield(int32 gridid, char* fieldname, int32 start[],
                      int32 stride[], int32 edge[], void *buffer);
    intn  SWclose(int32 fid);
    int32 SWinqswath(char *filename, char *swathlist, int32 *strbufsize);
    int32 SWopen(char *name, intn access);
"""

SOURCE = """
    #include "mfhdf.h"
    #include "HdfEosDef.h"
"""

ffi = FFI()
ffi.cdef(CDEF)

libraries = config.hdfeos_libs
libraries.extend(config.hdf4_libs)
library_dirs = config.library_config(libraries)

_lib = ffi.verify(SOURCE,
                  ext_package='pyhdfeos',
                  libraries=libraries,
                  include_dirs=config.include_dirs,
                  library_dirs=library_dirs,
                  modulename=config._create_modulename("_hdfeos",
                                                       CDEF,
                                                       SOURCE,
                                                       sys.version))

def _handle_error(status):
    if status < 0:
        raise IOError("Library routine failed.")

DFACC_READ = 1
DFACC_WRITE = 2
DFACC_CREATE = 4
HDFE_CENTER = 0
HDFE_CORNER = 1
HDFE_NENTDIM = 0
HDFE_NENTFLD = 4
HDFE_GD_UL = 0
HDFE_GD_UR = 1
HDFE_GD_LL = 2
HDFE_GD_LR = 3
DFNT_FLOAT = 5

number_type_dict = {
                    3: np.uint16,
                    4: np.int8,
                    5: np.float32,
                    6: np.float64,
                    20: np.int8,
                    21: np.uint8,
                    22: np.int16,
                    23: np.uint16,
                    24: np.int32,
                    25: np.uint32,
                    26: np.int64,
                    27: np.uint64,
        }
cast_string_dict = {
                    3: "unsigned short *",
                    4: "signed char *",
                    5: "float *",
                    6: "double *",
                    20: "signed char *",
                    21: "unsigned char *",
                    22: "short int *",
                    23: "unsigned short int *",
                    24: "int *",
                    25: "unsigned int *",
                    26: "long long int",
                    27: "unsigned long long *",
        }

def ehidinfo(fid):
    """Get HDF file IDs.

    This function wraps the HDF-EOS EHidinfo library function.

    Parameters
    ----------
    gdfid : int
        Grid file id.
    
    Returns
    -------
    hdfid : int
        HDF file ID as returned by Hopen
    sdid : int
        SD interface ID as returned by SDstart

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    hdfidp = ffi.new("int32 *")
    sdidp = ffi.new("int32 *")
    status = _lib.EHidinfo(fid, hdfidp, sdidp)
    return hdfidp[0], sdidp[0]

def gdattach(gdfid, gridname):
    """Attach to an existing grid structure.

    This function wraps the HDF-EOS GDattach library function.

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

def gdattrinfo(grid_id, attr_name):
    """return information about a grid attribute

    Parameters
    ----------
    grid_id : int
        grid identifier
    attr_name : str
        attribute name

    Returns
    -------
    number_type : int
        number type of attribute
    count : int
        number of total bytes in attribute

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    number_type_p = ffi.new("int32 *")
    count_p = ffi.new("int32 *")
    status = _lib.GDattrinfo(grid_id, attr_name.encode(),
                             number_type_p, count_p)
    _handle_error(status)

    return number_type_p[0], count_p[0]

def gdblksomoffset(grid_id):
    """read SOM block offsets

    Parameters
    ----------
    grid_id : int
        grid identifier

    Returns
    -------
    offsets : ndarray
        SOM block offsets

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    _, _, _, projparms = gdprojinfo(grid_id)
    num_offsets = int(projparms[11])
    offset = np.zeros(num_offsets - 1, dtype=np.float32)
    offsetp = ffi.cast("float32 *", offset.ctypes.data)
    status = _lib.GDblkSOMoffset(grid_id, offsetp, num_offsets - 1,
                                  'r'.encode())
    _handle_error(status)
 
    return offset

def gdclose(gdfid):
    """Close an HDF-EOS file.

    This function wraps the HDF-EOS GDclose library function.

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

def gddetach(grid_id):
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

def gdgridinfo(grid_id):
    """Return information about a grid structure.

    Parameters
    ----------
    grid_id : int
        Grid identifier.

    Returns
    -------
    xdimsize, ydimsize : int
        shape of the grid
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

    upleft = np.zeros(2, dtype=np.float64)
    upleft[0] = upleft_buffer[0]
    upleft[1] = upleft_buffer[1]

    lowright = np.zeros(2, dtype=np.float64)
    lowright[0] = lowright_buffer[0]
    lowright[1] = lowright_buffer[1]

    return xdimsize[0], ydimsize[0], upleft, lowright

def gdfieldinfo(grid_id, fieldname):
    """Return information about a geolocation field or data field in a grid.

    This function wraps the HDF-EOS GDfieldinfo library function.

    Parameters
    ----------
    grid_id : int
        Grid identifier.
    fieldname : str
        field name

    Returns
    -------
    shape : tuple
        size of the field
    ntype : type
        datatype of the field
    dimlist : list
        list of dimensions
    """
    _, strbufsize = gdnentries(grid_id, HDFE_NENTDIM)
    strbufsize = max(1000, strbufsize)
    dimlist_buffer = ffi.new("char[]", b'\0' * (strbufsize + 1))

    rankp = ffi.new("int32 *")
    ntypep = ffi.new("int32 *")

    # Assume that no field has more than 8 dimensions.  Seems like a safe bet.
    # dimsp = ffi.new("int32[]", 8)
    dims = np.zeros(8, dtype=np.int32)
    dimsp = ffi.cast("int32 *", dims.ctypes.data)
    status = _lib.GDfieldinfo(grid_id, fieldname.encode(), rankp, dimsp,
                              ntypep, dimlist_buffer)  
    _handle_error(status)

    shape = []
    for j in range(rankp[0]):
        shape.append(dims[j])

    if sys.hexversion < 0x03000000:
        dimlist = ffi.string(dimlist_buffer).split(',')
    else:
        dimlist = ffi.string(dimlist_buffer).decode('ascii').split(',')

    return tuple(shape), ntypep[0], dimlist

def gdij2ll(projcode, zonecode, projparm, spherecode, xdimsize, ydimsize, upleft,
          lowright, row, col, pixcen, pixcnr):
    """Convert coordinates (i, j) to (longitude, latitude).

    This function wraps the HDF-EOS GDij2ll library function.

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

def gdinqfields(gridid):
    """Retrieve information about data fields defined in a grid.

    This function wraps the HDF-EOS GDinqfields library function.

    Parameters
    ----------
    grid_id : int
        grid identifier

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
    nfields, strbufsize = gdnentries(gridid, HDFE_NENTFLD)
    if nfields == 0:
        return [], None, None

    fieldlist_buffer = ffi.new("char[]", b'\0' * (strbufsize + 1))
    rank_buffer = ffi.new("int[]", nfields)
    numbertype_buffer = ffi.new("int[]", nfields)
    nfields2 = _lib.GDinqfields(gridid, fieldlist_buffer,
                                rank_buffer, numbertype_buffer)
    fieldlist = ffi.string(fieldlist_buffer).decode('ascii').split(',')
    if sys.hexversion < 0x03000000:
        fieldlist = ffi.string(fieldlist_buffer).split(',')
    else:
        fieldlist = ffi.string(fieldlist_buffer).decode('ascii').split(',')

    ranks = []
    numbertypes = []
    for j in range(len(fieldlist)):
        ranks.append(rank_buffer[j])
        numbertypes.append(numbertype_buffer[j])

    return fieldlist, ranks, numbertypes

def gdinqattrs(gridid):
    """Retrieve information about grid attributes.

    Parameters
    ----------
    grid_id : int
        grid identifier

    Returns
    -------
    attrlist : list
        list of attributes defined for the grid

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    strbufsize = ffi.new("int32 *")
    nattrs = _lib.GDinqattrs(gridid, ffi.NULL, strbufsize)
    if nattrs == 0:
        return []
    attr_buffer = ffi.new("char[]", b'\0' * (strbufsize[0] + 1))
    nattrs = _lib.GDinqattrs(gridid, attr_buffer, strbufsize)
    _handle_error(nattrs)
    if sys.hexversion < 0x03000000:
        attr_list = ffi.string(attr_buffer).split(',')
    else:
        attr_list = ffi.string(attr_buffer).decode('ascii').split(',')
    return attr_list

def gdinqdims(gridid):
    """Retrieve information about dimensions defined in a grid.

    This function wraps the HDF-EOS GDinqdims library function.

    Parameters
    ----------
    grid_id : int
        grid identifier

    Returns
    -------
    dimlist : list
        list of dimensions defined for the grid
    dimlens : ndarray
        corresponding length of each dimension

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    ndims, strbufsize = gdnentries(gridid, HDFE_NENTDIM)
    dim_buffer = ffi.new("char[]", b'\0' * (strbufsize + 1))
    dimlens = np.zeros(ndims, dtype=np.int32)
    dimlensp = ffi.cast("int32 *", dimlens.ctypes.data)
    status = _lib.GDinqdims(gridid, dim_buffer, dimlensp)
    _handle_error(status)
    dimlist = ffi.string(dim_buffer).decode('ascii').split(',')
    return dimlist, dimlens

def gdinqgrid(filename):
    """Retrieve grid structures defined in HDF-EOS file.

    This function wraps the HDF-EOS GDinqgrid library function.

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
    if ngrids == 0:
        return []
    gridbuffer = ffi.new("char[]", b'\0' * (strbufsize[0] + 1))
    ngrids = _lib.GDinqgrid(filename.encode(), gridbuffer, ffi.NULL)
    _handle_error(ngrids)
    if sys.hexversion < 0x03000000:
        gridlist = ffi.string(gridbuffer).split(',')
    else:
        gridlist = ffi.string(gridbuffer).decode('ascii').split(',')
    return gridlist

def gdnentries(gridid, entry_code):
    """Return number of specified objects in a grid.

    This function wraps the HDF-EOS GDnentries library function.

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
    strbufsizep = ffi.new("int32 *")
    nentries = _lib.GDnentries(gridid, entry_code, strbufsizep)

    # sometimes running this with HDFE_NENTDIM results in 0 for STRBUFSIZE.
    # This should never be correct, make it a minimum of 9 (for "YDim,XDim").
    if entry_code == HDFE_NENTDIM:
        strbufsize = max(9, strbufsizep[0])
    else:
        strbufsize = strbufsizep[0]
    return nentries, strbufsize

def gdopen(filename, access=DFACC_READ):
    """Opens or creates HDF file in order to create, read, or write a grid.
    
    This function wraps the HDF-EOS GDopen library function.

    Parameters
    ----------
    filename : str
        name of file
    access : int
        one of H5F_ACC_RDONLY, H5F_ACC_RDWR, or H5F_ACC_TRUNC

    Returns
    -------
    fid : int
        grid file ID handle
    """
    fid = _lib.GDopen(filename.encode(), access)
    _handle_error(fid)
    return fid

def gdorigininfo(grid_id):
    """Return grid pixel origin information.

    This function wraps the HDF-EOS GDorigininfo library function.

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

def gdpixreginfo(grid_id):
    """Return pixel registration information.

    This function wraps the HDF-EOS GDpixreginfo library function.

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

def gdprojinfo(grid_id):
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

def gdreadattr(gridid, attrname):
    """read grid attribute

    This function wraps the HDF-EOS library GDreadattr function.

    Parameters
    ----------
    grid_id : int
        grid identifier
    attrname : str
        attribute name

    Returns
    -------
    value : object
        grid attribute value

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    [ntype, count] = gdattrinfo(gridid, attrname)
    if ntype == 4:
        # char8
        buffer = ffi.new("char[]", b'\0' * (count + 1))
        status = _lib.GDreadattr(gridid, attrname.encode(), buffer)
        _handle_error(status)
        return ffi.string(buffer).decode('ascii')

    buffer = np.zeros(count, dtype=number_type_dict[ntype])
    pbuffer = ffi.cast(cast_string_dict[ntype], buffer.ctypes.data)

    status = _lib.GDreadattr(gridid, attrname.encode(), pbuffer)
    _handle_error(status)
    return buffer

def gdreadfield(gridid, fieldname, start, stride, edge):
    """read data from grid field

    This function wraps the HDF-EOS library GDreadfield function.

    Parameters
    ----------
    grid_id : int
        grid identifier
    fieldname : str
        attribute name
    start : array-like
        specifies starting location within each dimension
    stride : array-like
        specifies number of values to skip along each dimension
    edge : array-like
        specifies number of values to read along each dimension

    Returns
    -------
    data : ndarray
        data read from field

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    _, ntype, _  = gdfieldinfo(gridid, fieldname)
    shape = tuple([int(x) for x in edge])
    buffer = np.zeros(shape, dtype=number_type_dict[ntype])
    pbuffer = ffi.cast(cast_string_dict[ntype], buffer.ctypes.data)

    startp = ffi.new("int32 []", len(shape))
    stridep = ffi.new("int32 []", len(shape))
    edgep = ffi.new("int32 []", len(shape))

    for j in range(len(shape)):
        startp[j] = int(start[j])
        stridep[j] = int(stride[j])
        edgep[j] = int(edge[j])

    status = _lib.GDreadfield(gridid, fieldname.encode(), startp, stridep,
                              edgep, pbuffer)
    _handle_error(status)
    return buffer

def swclose(swfid):
    """Close an HDF-EOS file.

    This function wraps the HDF-EOS SWclose library function.

    Parameters
    ----------
    fid : int
        swath file id

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    status = _lib.SWclose(swfid)
    _handle_error(status)

def swinqswath(filename):
    """Retrieve swath structures defined in HDF-EOS file.

    This function wraps the HDF-EOS SWinqswath library function.

    Parameters
    ----------
    filename : str
        swath file name

    Returns
    -------
    swathlist : list
        List of swaths defined in HDF-EOS file.

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    strbufsize = ffi.new("int32 *")
    nswaths = _lib.SWinqswath(filename.encode(), ffi.NULL, strbufsize)
    if nswaths == 0:
        return []
    swathbuffer = ffi.new("char[]", b'\0' * (strbufsize[0] + 1))
    ngrids = _lib.SWinqswath(filename.encode(), swathbuffer, ffi.NULL)
    _handle_error(nswaths)
    if sys.hexversion < 0x03000000:
        swathlist = ffi.string(swathbuffer).split(',')
    else:
        swathlist = ffi.string(swathbuffer).decode('ascii').split(',')
    return swathlist

def swopen(filename, access=DFACC_READ):
    """Opens or creates HDF file in order to create, read, or write a swath.
    
    This function wraps the HDF-EOS SWopen library function.

    Parameters
    ----------
    filename : str
        name of file
    access : int
        one of H5F_ACC_RDONLY, H5F_ACC_RDWR, or H5F_ACC_TRUNC

    Returns
    -------
    fid : int
        swath file ID handle
    """
    fid = _lib.SWopen(filename.encode(), access)
    _handle_error(fid)
    return fid



