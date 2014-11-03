import platform

from cffi import FFI

ffi = FFI()
ffi.cdef("""
        typedef unsigned uintn;
        typedef int hid_t;
        typedef int herr_t;

        hid_t  HE5_GDattach(hid_t fid, char *gridname);
        herr_t HE5_GDclose(hid_t fid);
        herr_t HE5_GDdetach(hid_t gridid);
        long   HE5_GDinqgrid(const char *filename, char *gridlist, long *strbufsize);
        hid_t  HE5_GDopen(const char *filename, uintn access);
        /*int HE5_EHHEisHE5(char *filename);*/
        """)

if platform.system().startswith('Linux'):
    if platform.linux_distribution() == ('Fedora', '20', 'Heisenbug'):
        libraries=['hdfeos', 'Gctp', 'mfhdf', 'df', 'jpeg', 'z']
    else:
        # Linux Mint 17?
        libraries=['he5_hdfeos', 'gctp', 'hdf5_hl', 'hdf5', 'z']

else:
    libraries=['hdfeos', 'Gctp', 'mfhdf', 'df', 'jpeg', 'z']
_lib = ffi.verify("""
        #include "HE5_HdfEosDef.h"
        """,
        libraries=libraries,
        include_dirs=['/usr/include/hdf-eos5',
                      '/opt/local/include',
                      '/usr/local/include'],
        library_dirs=['/usr/lib', '/opt/local/lib', '/usr/local/lib', '/usr/lib/x86_64-linux-gnu'])

H5F_ACC_RDONLY = 0x0000

def _handle_error(status):
    if status < 0:
        raise IOError("Library routine failed.")

def ehheish5(filename):
    """Determine if the input file type is HDF-EOS5.

    This function wraps the HDF-EOS5 HE5_EHHEisHE5 library function.

    Parameters
    ----------
    filename : str
        name of file

    Returns
    -------
    yesno : bool
        1 if file is HDF-EOS5, 0 if not HDF-EOS5, -1 otherwise.
    """
    return _lib.HE5_EHHEisHE5(filename.encode())

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
    strbufsize = ffi.new("long *")
    ngrids = _lib.HE5_GDinqgrid(filename.encode(), ffi.NULL, strbufsize)
    gridbuffer = ffi.new("char[]", b'\0' * (strbufsize[0] + 1))
    ngrids = _lib.HE5_GDinqgrid(filename.encode(), gridbuffer, ffi.NULL)
    _handle_error(ngrids)
    gridlist = ffi.string(gridbuffer).decode('ascii').split(',')
    return gridlist

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

