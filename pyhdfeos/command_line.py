import argparse

from .grids import GridFile

def dump_metadata():

    description='Print HDF-EOS grid metadata.'
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('filename')

    args = parser.parse_args()
    gdf = GridFile(args.filename)
    print(gdf)


