import argparse

from .grids import GridFile
from .swaths import SwathFile


def dump_metadata():

    description = 'Print HDF-EOS grid metadata.'
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('filename')

    args = parser.parse_args()

    gdf = GridFile(args.filename)
    if len(gdf.grids) > 0:
        print(gdf)

    swf = SwathFile(args.filename)
    if len(swf.swaths) > 0:
        print(swf)
