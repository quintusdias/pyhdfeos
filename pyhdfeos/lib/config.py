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
        msg += "the LIBRARY_DIRS environment variable as specified in README.md."
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
    libraries = []

    # On Fedora, gctp is named libGctp, but on ubuntu variants, it is libgctp.
    suffix_list = ['a', 'so', 'dylib', 'dll']
    for libname in libraries:
        for library_dir in library_dir_candidates:
            for suffix in suffix_list:
                path = os.path.join(library_dir, 'lib' + libname + '.' + suffix)
                #print(path)
                if os.path.exists(path):
                    if library_dir not in library_dirs:
                        library_dirs.append(library_dir)

    return library_dirs

# Locations where we might look for HDF, HDF-EOS, and HDF-EOS5 libraries.
# Modify this for your local configuration.
include_dirs=['/usr/include/hdf',
              '/usr/include/x86_64-linux-gnu/hdf',
              '/usr/include/i386-linux-gnu/hdf',
              '/opt/local/include',
              '/usr/local/include',
              '/usr/include/hdf-eos5']

library_dir_candidates = ['/usr/lib', '/usr/lib/hdf', '/usr/lib64/hdf',
                          '/usr/lib/i386-linux-gnu', '/usr/lib/x86_64-linux-gnu',
                          '/usr/local/lib', '/opt/local/lib']

hdf4_libs = ['mfhdf', 'df', 'jpeg', 'z']

true_gctp_lib = locate_gctp(library_dir_candidates)
hdfeos_libs = ['hdfeos', true_gctp_lib]

hdfeos5_libs = ['he5_hdfeos', true_gctp_lib]
hdfeos5_libs.extend(['hdf5_hl', 'hdf5', 'z'])
