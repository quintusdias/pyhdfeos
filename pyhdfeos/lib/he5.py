import platform
import sys

import numpy as np

from .core import decode_comma_delimited_ffi_string, ffi, _lib

H5F_ACC_RDONLY = 0x0000

HE5_HDFE_NENTDIM = 0
HE5_HDFE_NENTMAP = 1
HE5_HDFE_NENTIMAP = 2
HE5_HDFE_NENTGFLD = 3
HE5_HDFE_NENTDFLD = 4

number_type_dict = {0: np.int32,
                    1: np.uint32,
                    2: np.int16,
                    3: np.uint16,
                    4: np.int8,
                    5: np.uint8,
                    6: np.int64,
                    8: np.int64,
                    9: np.uint64,
                    10: np.float32,
                    11: np.float64,
                    57: np.str}


cast_string_dict = {np.int32: "int *",
                    np.uint32: "unsigned int *",
                    np.int16: "short int *",
                    np.uint16: "unsigned short int *",
                    np.int8: "signed char *",
                    np.uint8: "unsigned char *",
                    np.int64: "long long int *",
                    np.uint64: "unsigned long long int *",
                    np.float32: "float *",
                    np.float64: "double *",
                    np.str: "char *"}


def _handle_error(status):
    if status < 0:
        raise IOError("Library routine failed.")


def ehheishe5(filename):
    """Determine if the input file type is HDF-EOS5

    This function wraps the HDF-EOS5 HE5_EHHEisHE5 library function.

    Parameters
    ----------
    filename : str
        name of the file

    Returns
    -------
    True if the file is HDF-EOS5, False is not.
    """
    status = _lib.HE5_EHHEisHE5(filename.encode())
    _handle_error(status)
    if status == 1:
        return True
    else:
        return False


def gdattach(gdfid, gridname):
    """Attach to an existing grid within the file.

    This function wraps the HDF-EOS5 HE5_GDattach library function.

    Parameters
    ----------
    gdfid : int
        grid file id
    gridname : str
        name of grid to be attached

    Returns
    -------
    grid_id : int
        grid identifier
    """
    return _lib.HE5_GDattach(gdfid, gridname.encode())


def gdattrinfo(grid_id, attrname):
    """return information about a grid attribute

    Parameters
    ----------
    grid_id : int
        grid identifier
    attrname : str
        attribute name

    Returns
    -------
    ntype : int
        numpy datatype of attribute, see Appendix A in "HDF-EOS Interface Based
        on HDF5, Volume 2: Function Reference Guide"
    count : int
        number of attribute elements
    """
    ntypep = ffi.new("hid_t *")
    countp = ffi.new("hsize_t *")
    status = _lib.HE5_GDattrinfo(grid_id, attrname.encode(), ntypep, countp)
    _handle_error(status)

    return number_type_dict[ntypep[0]], countp[0]


def gdclose(fid):
    """Closes the HDF-EOS grid file.

    This function wraps the HDF-EOS5 HE5_GDclose library function.

    Parameters
    ----------
    fid : int
        grid file ID
    """
    status = _lib.HE5_GDclose(fid)
    _handle_error(status)


def gddetach(grid_id):
    """Detach from grid structure.

    This function wraps the HDF-EOS5 HE5_GDdetach library function.

    Parameters
    ----------
    grid_id : int
        Grid identifier.
    """
    status = _lib.HE5_GDdetach(grid_id)
    _handle_error(status)


def gdfieldinfo(grid_id, fieldname):
    """Return information about a geolocation field or data field in a grid.

    This function wraps the HDF-EOS5 HE5_GDfieldinfo library function.

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
    dimlist, maxdimlist : list
        list of dimensions
    """
    _, strbufsize = gdnentries(grid_id, HE5_HDFE_NENTDIM)
    dimlistb = ffi.new("char[]", b'\0' * (strbufsize + 1))
    maxdimlistb = ffi.new("char[]", b'\0' * (strbufsize + 1))

    rankp = ffi.new("int *")
    ntypep = ffi.new("hid_t *")

    # Assume that no field has more than 8 dimensions.  Seems like a safe bet.
    dims = np.zeros(8, dtype=np.uint64)
    dimsp = ffi.cast("hsize_t *", dims.ctypes.data)

    status = _lib.HE5_GDfieldinfo(grid_id, fieldname.encode(), rankp, dimsp,
                                  ntypep, dimlistb, maxdimlistb)
    _handle_error(status)

    shape = []
    for j in range(rankp[0]):
        shape.append(dimsp[j])

    dimlist = decode_comma_delimited_ffi_string(ffi.string(dimlistb))
    maxdimlist = decode_comma_delimited_ffi_string(ffi.string(maxdimlistb))

    return tuple(shape), number_type_dict[ntypep[0]], dimlist, maxdimlist


def gdgridinfo(grid_id):
    """Return information about a grid structure.

    This function wraps the HDF-EOS5 HE5_GDgridinfo library function.

    Parameters
    ----------
    grid_id : int
        Grid identifier.

    Returns
    -------
    shape : tuple
        Number of rows, columns in grid.
    upleft, lowright : np.float64[2]
        Location in meters of upper left, lower right corners.
    """
    xdimsize = ffi.new("long *")
    ydimsize = ffi.new("long *")

    upleft = np.zeros(2, dtype=np.float64)
    upleftp = ffi.cast("double *", upleft.ctypes.data)
    lowright = np.zeros(2, dtype=np.float64)
    lowrightp = ffi.cast("double *", lowright.ctypes.data)

    status = _lib.HE5_GDgridinfo(grid_id, xdimsize, ydimsize, upleftp,
                                 lowrightp)
    _handle_error(status)

    return xdimsize[0], ydimsize[0], upleft, lowright


def gdij2ll(projcode, zonecode, projparm, spherecode, xdimsize, ydimsize,
            upleft, lowright, row, col, pixcen, pixcnr):
    """Convert coordinates (i, j) to (longitude, latitude).

    This function wraps the HDF-EOS5 HE5_GDij2ll library function.

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
    """
    # This might be wrong on 32-bit machines.
    if sys.maxsize < 2**32 and platform.system().startswith('Linux'):
        row = row.astype(np.int32)
        col = col.astype(np.int32)
    else:
        row = row.astype(np.int64)
        col = col.astype(np.int64)

    longitude = np.zeros(col.shape, dtype=np.float64)
    latitude = np.zeros(col.shape, dtype=np.float64)
    upleftp = ffi.cast("double *", upleft.ctypes.data)
    lowrightp = ffi.cast("double *", lowright.ctypes.data)
    projparmp = ffi.cast("double *", projparm.ctypes.data)
    colp = ffi.cast("long *", col.ctypes.data)
    rowp = ffi.cast("long *", row.ctypes.data)
    longitudep = ffi.cast("double *", longitude.ctypes.data)
    latitudep = ffi.cast("double *", latitude.ctypes.data)
    status = _lib.HE5_GDij2ll(projcode, zonecode, projparmp, spherecode,
                              xdimsize, ydimsize, upleftp, lowrightp, col.size,
                              rowp, colp, longitudep, latitudep,
                              pixcen, pixcnr)
    _handle_error(status)
    return longitude, latitude


def gdinqattrs(gridid):
    """Retrieve information about attributes for a specific grid.

    This function wraps the HDF-EOS5 HE5_GDinqattrs library function.

    Parameters
    ----------
    grid_id : int
        grid identifier

    Returns
    -------
    attrlist : list
        list of attributes defined for the grid
    """
    strbufsizep = ffi.new("long *")
    nattrs = _lib.HE5_GDinqattrs(gridid, ffi.NULL, strbufsizep)
    if nattrs == 0:
        return []
    strbufsize = strbufsizep[0]
    strbufsize = max(strbufsize, 1000)
    attr_buffer = ffi.new("char[]", b'\0' * (strbufsize + 1))
    nattrs = _lib.HE5_GDinqattrs(gridid, attr_buffer, strbufsizep)
    _handle_error(nattrs)
    attr_list = decode_comma_delimited_ffi_string(ffi.string(attr_buffer))
    return attr_list


def gdinqdims(gridid):
    """Retrieve information about dimensions defined in a grid.

    This function wraps the HDF-EOS5 HE5_GDinqdims library function.

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
    """
    ndims, strbufsize = gdnentries(gridid, HE5_HDFE_NENTDIM)
    dim_buffer = ffi.new("char[]", b'\0' * (strbufsize + 1))
    dimlens = np.zeros(ndims, dtype=np.uint64)
    dimlensp = ffi.cast("unsigned long long *", dimlens.ctypes.data)
    status = _lib.HE5_GDinqdims(gridid, dim_buffer, dimlensp)
    _handle_error(status)
    dimlist = ffi.string(dim_buffer).decode('ascii').split(',')
    return dimlist, dimlens


def gdinqfields(gridid):
    """Retrieve information about the data fields defined in a grid.

    This function wraps the HDF-EOS5 HE5_GDinqfields library function.

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
    """
    nfields, strbufsize = gdnentries(gridid, HE5_HDFE_NENTDFLD)
    if nfields == 0:
        return [], None, None

    fieldlist_buffer = ffi.new("char[]", b'\0' * (strbufsize + 1))
    ranks = np.zeros(nfields, dtype=np.int32)
    rankp = ffi.cast("int *", ranks.ctypes.data)
    numbertypes = np.zeros(nfields, dtype=np.int32)
    numbertypep = ffi.cast("hid_t *", numbertypes.ctypes.data)
    nfields = _lib.HE5_GDinqfields(gridid, fieldlist_buffer,
                                   rankp, numbertypep)
    _handle_error(nfields)
    fieldlist = decode_comma_delimited_ffi_string(ffi.string(fieldlist_buffer))

    return fieldlist, ranks, numbertypes


def gdinqgrid(filename):
    """Retrieve names of grids defined in HDF-EOS5 file.

    This function wraps the HDF-EOS5 HE5_GDinqgrid library function.

    Parameters
    ----------
    filename : str
        name of file

    Returns
    -------
    gridlist : list
        List of grids defined in HDF-EOS file.
    """
    strbufsizep = ffi.new("long *")
    ngrids = _lib.HE5_GDinqgrid(filename.encode(), ffi.NULL, strbufsizep)
    if ngrids == 0:
        return []
    gridbuffer = ffi.new("char[]", b'\0' * (strbufsizep[0] + 1))
    ngrids = _lib.HE5_GDinqgrid(filename.encode(), gridbuffer, ffi.NULL)
    _handle_error(ngrids)
    if sys.hexversion < 0x03000000:
        gridlist = ffi.string(gridbuffer).split(',')
    else:
        gridlist = ffi.string(gridbuffer).decode('ascii').split(',')
    return gridlist


def gdinqlocattrs(gridid, fieldname):
    """Retrieve information about grid field attributes.

    This function wraps the HDF-EOS5 HE5_GDinqlocattrs library function.

    Parameters
    ----------
    grid_id : int
        grid identifier
    fieldname : str
        retrieve attribute names for this field

    Returns
    -------
    attrlist : list
        list of attributes defined for the grid
    """
    strbufsize = ffi.new("long *")
    nattrs = _lib.HE5_GDinqlocattrs(gridid, fieldname.encode(),
                                    ffi.NULL, strbufsize)
    if nattrs == 0:
        return []
    attr_buffer = ffi.new("char[]", b'\0' * (strbufsize[0] + 1))
    nattrs = _lib.HE5_GDinqlocattrs(gridid, fieldname.encode(),
                                    attr_buffer, strbufsize)
    _handle_error(nattrs)
    attr_list = decode_comma_delimited_ffi_string(ffi.string(attr_buffer))
    return attr_list


def gdlocattrinfo(grid_id, fieldname, attrname):
    """return information about a grid field attribute

    Parameters
    ----------
    grid_id : int
        grid identifier
    fieldname : str
        attribute name
    attrname : str
        attribute name

    Returns
    -------
    numbertype : type
        numpy datatype of the attribute
    count : int
        number of attribute elements
    """
    ntypep = ffi.new("hid_t *")
    countp = ffi.new("hsize_t *")
    status = _lib.HE5_GDlocattrinfo(grid_id,
                                    fieldname.encode(), attrname.encode(),
                                    ntypep, countp)
    _handle_error(status)

    return number_type_dict[ntypep[0]], countp[0]


def gdnentries(gridid, entry_code):
    """Return number of specified objects in a grid.

    This function wraps the HDF-EOS5 HE5_GDnentries library function.

    Parameters
    ----------
    grid_id : int
        Grid identifier.
    entry_code : int
        Entry code, either HE5_HDFE_NENTDIM or HE5_HDFE_NENTDFLD

    Returns
    -------
    nentries, strbufsize : int
       Number of specified entries, number of bytes in descriptive strings.
    """
    strbufsizep = ffi.new("long *")
    nentries = _lib.HE5_GDnentries(gridid, entry_code, strbufsizep)

    # Sometimes running this with HDFE_NENTDIM results in too small a value.
    # Since there's no real reason for a python user to be directly interested
    # in this value, we'll make it a minimum size of 100.
    if entry_code == HE5_HDFE_NENTDIM:
        strbufsize = max(100, strbufsizep[0])
    else:
        strbufsize = strbufsizep[0]
    return nentries, strbufsize


def gdopen(filename, access=H5F_ACC_RDONLY):
    """Opens or creates HDF file in order to create, read, or write a grid.

    This function wraps the HDF-EOS5 HE5_GDopen library function.

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
    fid = _lib.HE5_GDopen(filename.encode(), access)
    _handle_error(fid)
    return fid


def gdorigininfo(grid_id):
    """Return grid pixel origin information.

    This function wraps the HDF-EOS5 HE5_GDorigininfo library function.

    Parameters
    ----------
    grid_id : int
        Grid identifier.

    Returns
    -------
    origincode : int
        Origin code.
    """
    origincode = ffi.new("int *")
    status = _lib.HE5_GDorigininfo(grid_id, origincode)
    _handle_error(status)
    return origincode[0]


def gdpixreginfo(grid_id):
    """Return pixel registration information.

    This function wraps the HDF-EOS5 HE5_GDpixreginfo library function.

    Parameters
    ----------
    grid_id : int
        Grid identifier.

    Returns
    -------
    pixregcode : int
        Pixel registration code.
    """
    pixregcode = ffi.new("int *")
    status = _lib.HE5_GDpixreginfo(grid_id, pixregcode)
    _handle_error(status)
    return pixregcode[0]


def gdprojinfo(grid_id):
    """Return projection information about grid.

    This function wraps the HDF-EOS5 HE5_GDprojinfo library function.

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
    """
    projcode = ffi.new("int *")
    zonecode = ffi.new("int *")
    spherecode = ffi.new("int *")
    projparm = np.zeros(13, dtype=np.float64)
    projparmp = ffi.cast("double *", projparm.ctypes.data)
    status = _lib.HE5_GDprojinfo(grid_id, projcode, zonecode, spherecode,
                                 projparmp)
    _handle_error(status)

    return projcode[0], zonecode[0], spherecode[0], projparm


def gdreadattr(gridid, attrname):
    """read grid attribute

    This function wraps the HDF-EOS5 HE5_GDreadattr library function.

    Parameters
    ----------
    grid_id : int
        grid identifier
    attrname : str
        attribute name

    Returns
    -------
    value : object
        grid field attribute value
    """
    [dtype, count] = gdattrinfo(gridid, attrname)
    if dtype is np.str:
        # buffer = ffi.new("char[]", b'\0' * (count + 1))
        buffer = ffi.new("char[]", b'\0' * (1000 + 1))
        status = _lib.HE5_GDreadattr(gridid, attrname.encode(), buffer)
        _handle_error(status)
        return ffi.string(buffer).decode('ascii')

    buffer = np.zeros(count, dtype=dtype)
    bufferp = ffi.cast(cast_string_dict[dtype], buffer.ctypes.data)

    status = _lib.HE5_GDreadattr(gridid, attrname.encode(), bufferp)
    _handle_error(status)

    if count == 1:
        # present as a scalar rather than an array.
        buffer = buffer[0]
    return buffer


def gdreadlocattr(gridid, fieldname, attrname):
    """read grid field attribute

    This function wraps the HDF-EOS5 library HE5_GDreadlocattr function.

    Parameters
    ----------
    grid_id : int
        grid identifier
    fieldname : str
        name of grid field
    attrname : str
        attribute name

    Returns
    -------
    value : object
        grid field attribute value
    """
    [dtype, count] = gdlocattrinfo(gridid, fieldname, attrname)
    if dtype is np.str:
        buffer = ffi.new("char[]", b'\0' * (1000 + 1))
        status = _lib.HE5_GDreadlocattr(gridid,
                                        fieldname.encode(), attrname.encode(),
                                        buffer)
        _handle_error(status)
        return ffi.string(buffer).decode('ascii')

    buffer = np.zeros(count, dtype=dtype)
    bufferp = ffi.cast(cast_string_dict[dtype], buffer.ctypes.data)

    status = _lib.HE5_GDreadlocattr(gridid,
                                    fieldname.encode(), attrname.encode(),
                                    bufferp)
    _handle_error(status)

    if count == 1:
        # present as a scalar rather than an array.
        buffer = buffer[0]
    return buffer


def gdreadfield(gridid, fieldname, start, stride, edge):
    """read data from grid field

    This function wraps the HDF-EOS5 library HE5_GDreadfield function.

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
    """
    info = gdfieldinfo(gridid, fieldname)
    dtype = info[1]
    shape = tuple([int(x) for x in edge])
    buffer = np.zeros(shape, dtype=dtype)
    pbuffer = ffi.cast(cast_string_dict[dtype], buffer.ctypes.data)

    startp = ffi.new("const hsize_t []", len(shape))
    stridep = ffi.new("const hsize_t []", len(shape))
    edgep = ffi.new("const hsize_t []", len(shape))

    for j in range(len(shape)):
        startp[j] = np.uint64(start[j])
        stridep[j] = np.uint64(stride[j])
        edgep[j] = np.uint64(edge[j])

    status = _lib.HE5_GDreadfield(gridid, fieldname.encode(), startp, stridep,
                                  edgep, pbuffer)
    _handle_error(status)
    return buffer
