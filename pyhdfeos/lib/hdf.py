"""
Interface for HDF4 library.  Need this in order to access HDF-EOS2 field
attributes.
"""
import sys

import numpy as np
from cffi import FFI

from . import config

DFACC_READ = 1

DFNT_CHAR = 4
DFNT_FLOAT = 5
DFNT_FLOAT64 = 6
DFNT_INT8 = 20
DFNT_UINT8 = 21
DFNT_INT16 = 22
DFNT_UINT16 = 23
DFNT_INT32 = 24
DFNT_UINT32 = 25

DFTAG_NDG = 720
DFTAG_VG = 1965

CDEF = """
    typedef short int int16;
    typedef unsigned short int uint16;
    typedef int int32;
    typedef int intn;
    int32 Hopen(const char *path, intn acc_mode, int16 ndds);
    intn Hclose(int32 file_id);
    intn SDattrinfo(int obj_id, int32 idx, char *name, int32 *dtype,
                    int32 *count);
    intn SDendaccess(int32 sds_id);
    intn SDgetinfo(int32 sdsid, char *name, int32 *rank,
                   int32 dimsizes[], int32 *datatype, int32 *nattrs);
    int32 SDnametoindex(int32 sdid, char *sds_name);
    int32 SDreftoindex(int32 sd_id, int32 sds_ref);
    int32 SDselect(int32 sdid, int32 idx);
    intn SDreadattr(int32 obj_id, int32 idx, void *buffer);
    intn SDend(int32 fid);
    int32 SDstart(char *filename, int32 access_mode);
    int32 Vattach(int32 fid, int32 vgroup_ref, char *access);
    int32 Vdetach(int32 vgroup_id);
    intn Vend(int32 fid);
    int32 Vfind(int32 fid, char *vgroup_name);
    int32 Vgetname(int32 vgroup_id, char *vgroup_name);
    int32 Vgetnamelen(int32 vgroup_id, uint16 *namelen);
    int32 Vgettagrefs(int32 vgroup_id, int32 tags[], int32 refs[],
                      int32 npairs);
    int32 Vntagrefs(int32 vgroup_id);
    intn Vstart(int32 fid);
"""

SOURCE = """
    #include "hdf.h"
    #include "mfhdf.h"
"""

ffi = FFI()
ffi.cdef(CDEF)

_lib = ffi.verify(SOURCE,
                  libraries=config.hdf4_libraries,
                  include_dirs=config.include_dirs,
                  library_dirs=config.library_dirs,
                  extra_link_args=config.extra_link_args,
                  modulename=config._create_modulename("_hdf4",
                                                       CDEF,
                                                       SOURCE,
                                                       sys.version))

# Map the HDF4 datatypes to numpy.
hdf4_to_np = {DFNT_INT8: np.int8,
              DFNT_UINT8: np.uint8,
              DFNT_INT16: np.int16,
              DFNT_UINT16: np.uint16,
              DFNT_INT32: np.int32,
              DFNT_UINT32: np.uint32,
              DFNT_FLOAT: np.float32,
              DFNT_FLOAT64: np.float64}


def _handle_error(status):
    if status < 0:
        raise IOError("Library routine failed.")


def sdattrinfo(obj_id, idx):
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
    _handle_error(status)
    name = ffi.string(namebuffer).decode('ascii')
    return name, datatypep[0], countp[0]


def sdendaccess(sds_id):
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


def sdreftoindex(sd_id, sds_ref):
    """
    return the index of a data set given the reference number

    Parameters
    ----------
    sd_id : int
        SD interface identifier
    sds_ref : int
        reference number of the dataset

    Returns
    -------
    idx : int
        index of the dataset
    """
    idx = _lib.SDreftoindex(sd_id, sds_ref)
    _handle_error(idx)
    return(idx)


def sdgetinfo(sdsid):
    """Return the name, dimension sizes, datatype, number of attributes

    Parameters
    ----------
    sdsid : int
        data set identifier

    Returns
    -------
    name : str
        name of the data set
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
    return name, dimsizes, datatypep[0], numattrsp[0]


def hclose(fid):
    """Closes HDF data file.

    Parameters
    ----------
    fid : int
        file identifier
    """
    status = _lib.Hclose(fid)
    _handle_error(status)


def hopen(filename):
    """Open or create HDF data file.

    Parameters
    ----------
    filename : str
        file name

    Returns
    -------
    fid : int32
        file identifier
    """
    fid = _lib.Hopen(filename.encode(), DFACC_READ, 0)
    _handle_error(fid)
    return fid


def sdreadattr(obj_id, idx):
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
    name, dtype, count = sdattrinfo(obj_id, idx)
    if dtype == DFNT_CHAR:
        buffer = ffi.new("char[]", b'\0' * (count + 1))
        status = _lib.SDreadattr(obj_id, idx, buffer)
        _handle_error(status)
        return ffi.string(buffer).decode('ascii')

    buffer = np.zeros(count, dtype=hdf4_to_np[dtype])
    pbuffer = ffi.cast("void *", buffer.ctypes.data)

    status = _lib.SDreadattr(obj_id, idx, pbuffer)
    _handle_error(status)
    if count == 1:
        buffer = buffer[0]
    return buffer


def sdselect(sdid, idx):
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


def sdend(sd_id):
    """
    close scientific dataset

    Parameters
    ----------
    sd_id : int
        dataset (file) identifier
    """
    status = _lib.SDend(sd_id)
    _handle_error(status)


def sdstart(filename):
    """
    Parameters
    ----------
    filename : str
        file name

    Returns
    -------
    sd_id : int
        dataset (file) identifier
    """
    return _lib.SDstart(filename.encode(), DFACC_READ)


def vattach(fid, vgroup_ref):
    """
    initiate access to a new or existing group

    Parameters
    ----------
    fid : int
        file identifier
    vgroup_ref : int
        reference number for the vgroup

    Returns
    -------
    vg_id : int
        vgroup identifier
    """
    vg_id = _lib.Vattach(fid, vgroup_ref, "r".encode())
    _handle_error(vg_id)
    return vg_id


def vdetach(vg_id):
    """
    terminate access to a vgroup

    Parameters
    ----------
    vg_id : int
        vgroup identifier
    """
    status = _lib.Vdetach(vg_id)
    _handle_error(status)


def vend(fid):
    """
    close vgroup interface

    Parameters
    ----------
    fid : int
        file identifier
    """
    status = _lib.Vend(fid)
    _handle_error(status)


def vfind(fid, group_name):
    """
    return the reference number of a vgroup given the name

    Parameters
    ----------
    fid : int
        file identifier
    group_name : str
        name of the group

    Returns
    -------
    ref_id : group ID
       reference number of the group
    """
    ref_id = _lib.Vfind(fid, group_name.encode())
    if ref_id == 0:
        raise IOError("Library routine failed.")
    return ref_id


def vgetname(vgroup_id):
    """
    retrieve the name of a vgroup
    """
    n = vgetnamelen(vgroup_id)
    namebuffer = ffi.new("char[]", b'\0' * n)
    status = _lib.Vgetname(vgroup_id, namebuffer)
    _handle_error(status)
    name = ffi.string(namebuffer).decode('ascii')
    return name


def vgetnamelen(vgroup_id):
    """
    retrieve the length of the name of a vgroup
    """
    countp = ffi.new("uint16 *")
    status = _lib.Vgetnamelen(vgroup_id, countp)
    _handle_error(status)
    return countp[0]


def vgettagrefs(vgroup_id, ntags=None):
    """
    retrieve the tag/reference pairs of the hdf objects belonging to a vgroup

    Parameters
    ----------
    vgroup_id : int
        vgroup identifier
    ntags : int
        number of tag/reference pairs

    Returns
    -------
    tagrefs : list
       list of tag/ref pairs
    """
    if ntags is None:
        ntags = vntagrefs(vgroup_id)
    tagp = ffi.new("int32[]", ntags)
    refp = ffi.new("int32[]", ntags)
    status = _lib.Vgettagrefs(vgroup_id, tagp, refp, ntags)
    _handle_error(status)
    lst = []
    for j in range(ntags):
        lst.append((tagp[j], refp[j]))
    return lst


def vntagrefs(vgroup_id):
    """
    return number of objects in a vgroup

    Parameters
    ----------
    vg_id : int
        vgroup identifier

    Returns
    -------
    ntagrefs : int
        number of objects in the vgroup
    """
    ntagrefs = _lib.Vntagrefs(vgroup_id)
    _handle_error(ntagrefs)
    return ntagrefs


def vstart(fid):
    """
    start Vgroup interface

    Parameters
    ----------
    fid : int
        file identifier
    """
    status = _lib.Vstart(fid)
    _handle_error(status)
