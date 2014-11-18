import numpy as np
from cffi import FFI

DFNT_CHAR = 4
DFNT_FLOAT = 5
DFNT_FLOAT64 = 6
DFNT_INT8 = 20
DFNT_UINT8 = 21
DFNT_INT16 = 22
DFNT_UINT16 = 23
DFNT_INT32 = 24
DFNT_UINT32 = 25

ffi = FFI()
ffi.cdef("""
        typedef int int32;
        typedef int intn;
        intn SDattrinfo(int obj_id, int32 idx, char *name, int32 *dtype,
                        int32 *count);
        intn SDendaccess(int32 sds_id);
        intn SDgetinfo(int32 sdsid, char *name, int32 *rank,
                       int32 dimsizes[], int32 *datatype, int32 *nattrs);
        int32 SDnametoindex(int32 sdid, char *sds_name);
        int32 SDselect(int32 sdid, int32 idx);
        intn SDreadattr(int32 obj_id, int32 idx, void *buffer);
        """
)

_lib = ffi.verify("""
        #include "mfhdf.h"
        """,
        libraries=['mfhdf', 'df', 'jpeg', 'z'],
        include_dirs=['/usr/include/hdf',
                      '/usr/include/x86_64-linux-gnu/hdf',
                      '/opt/local/include',
                      '/usr/local/include'],
        library_dirs=['/usr/lib/hdf', '/usr/lib64/hdf',
                      '/opt/local/lib', '/usr/local/lib'])


def _handle_error(status):
    if status < 0:
        raise IOError("Library routine failed.")

def attrinfo(obj_id, idx):
    """Retrieve information about an attribute.

    Parameters
    ----------
    obj_id : int
        identifier of the object to which the attribute is attached
    idx : int
        index of the attribute

    Returns
    -------
    name : str
        name of the attribute
    datatype : int
        data type of the attribute
    count : int
        total number of values in the attribute
    """
    namebuffer = ffi.new("char[]", b'\0' * 65)
    datatypep = ffi.new("int32 *")
    countp = ffi.new("int32 *")
    status = _lib.SDattrinfo(obj_id, idx, namebuffer, datatypep, countp)
    name = ffi.string(namebuffer).decode('ascii')
    return name, datatypep[0], countp[0]

def endaccess(sds_id):
    """Terminate access to a data set.

    Parameters
    ----------
    sds_id : int
        data set identifier

    """
    status = _lib.SDendaccess(sds_id)
    _handle_error(status)

def nametoindex(sdid, name):
    """Determine the index of a data set given its name.

    Parameters
    ----------
    sdid : int
        SD interface identifier
    sds_name : str
        name of the dataset

    Returns
    -------
    idx : int
        index of the dataset
    """
    idx = _lib.SDnametoindex(sdid, name.encode())
    _handle_error(idx)
    return idx


def getinfo(sdsid):
    """Return the name, rank, dimension sizes, datatype, number of attributes

    Parameters
    ----------
    sdsid : int
        data set identifier

    Returns
    -------
    name : str
        name of the data set
    rank : int
        number of dimensions in the data set
    dimsizes : ndarray
        array containing the size of each dimension in the data set
    data_type : int
        data type for the data stored in the data set
    num_attrs : int
        number of attributes for the data set

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    namebuffer = ffi.new("char[]", b'\0' * 65)
    rankp = ffi.new("int32 *")
    dimsizes = np.zeros(32, dtype=np.int32)
    dimsizesp = ffi.cast("int32 *", dimsizes.ctypes.data)
    datatypep = ffi.new("int32 *")
    numattrsp = ffi.new("int32 *")
    status = _lib.SDgetinfo(sdsid, namebuffer, rankp, dimsizesp,
                            datatypep, numattrsp)
    _handle_error(status)

    name = ffi.string(namebuffer).decode('ascii')
    rank = rankp[0]
    dimsizes = dimsizes[0:rank]
    return name, rank, dimsizes, datatypep[0], numattrsp[0]

def readattr(obj_id, idx):
    """read attribute

    This function wraps the HDF-EOS library SDreadattr function.

    Parameters
    ----------
    obj_id : int
        identifier of the object the attribute is attached to
    idx : int
        index of the attribute to be read

    Returns
    -------
    value : object
        attribute value

    Raises
    ------
    IOError
        If associated library routine fails.
    """
    name, dtype, count = attrinfo(obj_id, idx)
    if dtype == DFNT_CHAR:
        buffer = ffi.new("char[]", b'\0' * (count + 1))
        status = _lib.SDreadattr(obj_id, idx, buffer)
        _handle_error(status)
        return ffi.string(buffer).decode('ascii')

    if dtype == DFNT_INT8:
        buffer = np.zeros(count, dtype = np.int8)
    elif dtype == DFNT_UINT8:
        buffer = np.zeros(count, dtype = np.uint8)
    elif dtype == DFNT_INT16:
        buffer = np.zeros(count, dtype = np.int16)
    elif dtype == DFNT_UINT16:
        buffer = np.zeros(count, dtype = np.uint16)
    elif dtype == DFNT_INT32:
        buffer = np.zeros(count, dtype = np.int32)
    elif dtype == DFNT_UINT32:
        buffer = np.zeros(count, dtype = np.uint32)
    elif dtype == DFNT_FLOAT:
        buffer = np.zeros(count, dtype = np.float32)
    elif dtype == DFNT_FLOAT64:
        buffer = np.zeros(count, dtype = np.float64)
    pbuffer = ffi.cast("void *", buffer.ctypes.data)

    status = _lib.SDreadattr(obj_id, idx, pbuffer)
    _handle_error(status)
    if count == 1:
        buffer = buffer[0]
    return buffer

def select(sdid, idx):
    """Obtain the data set identifier

    Parameters
    ----------
    sd_id : int
        SD interface identifier
    idx : int
        index of the data set

    Returns
    -------
    sds_id : int
        data set identifier of the associated data set
    """
    sds_id = _lib.SDselect(sdid, idx)
    _handle_error(sds_id)
    return sds_id

