import os

def library_config(library_dir_candidates, library_name_candidates):
    """
    Determine library directories.

    Parameters
    ----------
    library_dir_candidates : list
        List of library directories to search.
    library_name_candidates : list
        List of libraries that must be found.
    """
    library_dirs = []
    libraries = []

    # On Fedora, gctp is named libGctp, but on ubuntu variants, it is libgctp.
    suffix_list = ['a', 'so', 'dylib', 'dll']
    for libname in library_name_candidates:
        for library_dir in library_dir_candidates:
            for suffix in suffix_list:
                path = os.path.join(library_dir, 'lib' + libname + '.' + suffix)
                if os.path.exists(path):
                    if libname.lower() not in [x.lower() for x in libraries]:
                        libraries.append(libname)
                    if library_dir not in library_dirs:
                        library_dirs.append(library_dir)

    return library_dirs, libraries
