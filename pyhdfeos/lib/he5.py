import platform

from cffi import FFI

ffi = FFI()
ffi.cdef("""
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

        typedef int hid_t;

        int HE5_EHHEisHE5(char *filename);
        """,
        libraries=libraries,
        include_dirs=['/usr/include/hdf-eos5',
                      '/opt/local/include',
                      '/usr/local/include'],
        library_dirs=['/usr/lib', '/opt/local/lib', '/usr/local/lib', '/usr/lib/x86_64-linux-gnu'])


def EHHEisHE5(filename):
    """Wrapper for HE5_EHHEisHE5 function.

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
