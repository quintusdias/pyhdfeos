import binascii
import os

# HDFEOS relies on GCTP, but sometimes this library is called "Gctp" and
# sometimes it is called "gctp" (Ubuntu derivatives).
def locate_gctp(library_dirs):
    """
    Debian systems (including Mint) have libGctp.  Most other linux systems
    plus macports have libgctp.  Need to find out which one to use.
    """
    true_gctp_lib = None
    libs = ['gctp', 'Gctp']
    suffix_list = ['so', 'dylib', 'dll', 'a']
    for library_dir in library_dirs:
        for libname in libs:
            for suffix in suffix_list:
                path = os.path.join(library_dir, 'lib' + libname + '.' + suffix)
                if os.path.exists(path):
                    true_gctp_lib = libname
                    return true_gctp_lib

    if true_gctp_lib is None:
        msg = "Could not locate gctp library.  Please specify a location with "
        msg += "the HDFEOS_ROOT environment variable as specified in README.md."
        raise RuntimeError(msg)
        return None

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
                path = os.path.join(library_dir, 'lib' + libname + '.' + suffix)
                if os.path.exists(path):
                    if library_dir not in library_dirs:
                        library_dirs.append(library_dir)

    return library_dirs

# Locations where we might look for HDF, HDF-EOS, and HDF-EOS5 libraries.
include_dirs=['/usr/include/hdf',
              '/usr/include/x86_64-linux-gnu/hdf',
              '/usr/include/i386-linux-gnu/hdf',
              '/opt/local/include',
              '/usr/local/include',
              '/usr/include/hdf-eos5']
if 'INCLUDE_DIRS' in os.environ:
    lst = os.environ['INCLUDE_DIRS'].split(':')
    lst.extend(include_dirs)
    include_dirs = lst

library_dir_candidates = ['/usr/lib',
                          '/usr/lib/hdf',
                          '/usr/lib64/hdf',
                          '/usr/lib/i386-linux-gnu',
                          '/usr/lib/x86_64-linux-gnu',
                          '/usr/local/lib',
                          '/opt/local/lib']
if 'LIBRARY_DIRS' in os.environ:
    lst = os.environ['LIBRARY_DIRS'].split(':')
    lst.extend(library_dir_candidates)
    library_dir_candidates = lst

hdf4_libs = ['mfhdf', 'df', 'jpeg', 'z']

true_gctp_lib = locate_gctp(library_dir_candidates)
hdfeos_libs = ['hdfeos', true_gctp_lib]

hdfeos5_libs = ['he5_hdfeos', true_gctp_lib]
hdfeos5_libs.extend(['hdf5_hl', 'hdf5', 'z'])

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
    return '_{0}_cffi_{1}{2}'.format(tag,k1, k2)

