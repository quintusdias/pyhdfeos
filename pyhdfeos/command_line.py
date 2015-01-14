import argparse

from .grids import GridFile
from .swaths import SwathFile
from .za import ZonalAverageFile
from .lib import hdf

def dump_zonal_average(filename):

    # See if it is an HDF4 file before trying zonal averages.  This way
    # we don't get the annoying HDF5 library error messages if/when it fails.
    try:
        fid = hdf.hopen(filename)
        hdf.hclose(fid)
        numzas = 0
    except IOError:
        # Ok, it's HDF5.
        zaf = ZonalAverageFile(filename)
        numzas = len(zaf.zas)
        if numzas > 0:
            print(zaf)

    return numzas

def dump_metadata():

    description = 'Print HDF-EOS grid metadata.'
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('filename')

    args = parser.parse_args()

    gdf = GridFile(args.filename)
    numgrids = len(gdf.grids)
    if numgrids > 0:
        print(gdf)

    swf = SwathFile(args.filename)
    numswaths = len(swf.swaths)
    if numswaths > 0:
        print(swf)

    numzas = dump_zonal_average(args.filename)

    if numgrids == 0 and numswaths == 0 and numzas == 0:
        print('No HDF-EOS structures detected.')
