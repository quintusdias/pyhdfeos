import argparse

from .grids import GridFile
from .swaths import SwathFile
from .za import ZonalAverageFile


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

    zaf = ZonalAverageFile(args.filename)
    numzas = len(zaf.zas)
    if numzas > 0:
        print(zaf)

    if numgrids == 0 and numswaths == 0 and numzas == 0:
        print('No HDF-EOS structures detected.')
