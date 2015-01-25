import argparse

from .grids import GridFile
from .swaths import SwathFile
from .za import ZonalAverageFile
from .lib import he4, he5


def dump_metadata():

    description = 'Print HDF-EOS grid metadata.'
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('filename')

    args = parser.parse_args()

    if not he4.ehheishe2(args.filename) and not he5.ehheishe5(args.filename):
        print('No HDF-EOS structures detected.')
        return

    gdf = GridFile(args.filename)
    numgrids = len(gdf.grids)
    if numgrids > 0:
        print(gdf)

    swf = SwathFile(args.filename)
    numswaths = len(swf.swaths)
    if numswaths > 0:
        print(swf)

    if he5.ehheishe5(args.filename):
        zaf = ZonalAverageFile(args.filename)
        numzas = len(zaf.zas)
        if numzas > 0:
            print(zaf)
