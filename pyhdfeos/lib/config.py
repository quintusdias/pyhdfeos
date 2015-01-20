import binascii
import os


def library_config(libraries):
    """
    Determine library directories where HDF4, HDFEOS, and HDFEOS5 can be found.

    Parameters
    ----------
    libraries : list
        List of libraries that must be found.
    """
    global library_dir_candidates
    library_dirs = []

    # On Fedora, gctp is named libGctp, but on ubuntu variants, it is libgctp.
    suffix_list = ['a', 'so', 'dylib', 'dll']
    for libname in libraries:
        for library_dir in library_dir_candidates:
            for suffix in suffix_list:
                if libname is None:
                    continue
                path = os.path.join(library_dir,
                                    'lib' + libname + '.' + suffix)
                if os.path.exists(path):
                    if library_dir not in library_dirs:
                        library_dirs.append(library_dir)

    return library_dirs


def _create_modulename(tag, cdef_sources, source, sys_version):
    """
    This is the same as CFFI's create modulename except we don't include the
    CFFI version.

    See https://caremad.io/2014/11/distributing-a-cffi-project/
    """
    key = '\x00'.join([sys_version[:3], source, cdef_sources])
    key = key.encode('utf-8')
    k1 = hex(binascii.crc32(key[0::2]) & 0xffffffff)
    k1 = k1.lstrip('0x').rstrip('L')
    k2 = hex(binascii.crc32(key[1::2]) & 0xffffffff)
    k2 = k2.lstrip('0').rstrip('L')
    return '_{0}_cffi_{1}{2}'.format(tag, k1, k2)

try:
    extra_link_args = os.environ['EXTRA_LINK_ARGS'].split()
except KeyError:
    extra_link_args = None

if extra_link_args is None:
    hdf4_libraries = ["mfhdf", "df", "jpeg", "z"]
    hdf5_libraries = ["hdf5_hl", "hdf5"]
    library_dirs = ["/usr/lib"]
else:
    hdf4_libraries = None
    hdf5_libraries = None
    library_dirs = None

include_dirs = ["/usr/include",
                "/usr/include/hdf"]
