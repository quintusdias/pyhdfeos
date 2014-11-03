import platform

from cffi import FFI

ffi = FFI()
ffi.cdef("""
        typedef unsigned uintn;
        typedef int hid_t;
        typedef int herr_t;

        herr_t HE5_GDclose(hid_t fid);
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
    """determine if the input file type is HDF-EOS5

    wrapper for HE5_EHHEisHE5 function

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

def gdclose(fid):
    """Closes the HDF-EOS grid file.

    Wrapper for HE5_GDclose function.

    Parameters
    ----------
    fid : int
        grid file ID
    """
    status = _lib.HE5_GDclose(fid)
    _handle_error(status)

def gdopen(filename, access=H5F_ACC_RDONLY):
    """opens or creates HDF file in order to create, read, or write a grid
    
    wrapper for HE5_GDopen function

    Parameters
    ----------
    filename : str
        name of file

    Returns
    -------
    fid : int
        grid file ID handle
    """
    fid = _lib.HE5_GDopen(filename.encode(), access)
    _handle_error(fid)
    return fid

