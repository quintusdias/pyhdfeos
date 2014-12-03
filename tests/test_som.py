import os
import unittest

import numpy as np

from pyhdfeos import GridFile, som, misr
from pyhdfeos.lib import he4

from . import fixtures

def print_ij(block, i, j):
    R2D = 57.2957795131
    somx, somy = misr.inv(block+1, i, j)
    lon, lat = som.inv(somx, somy)
    lon *= R2D
    lat *= R2D
    print("({0} {1} {2}):  {3:.3f} {4:.3f}".format(block, i, j, lat, lon))

class TestSuite(unittest.TestCase):


    @unittest.skipIf('HDFEOS_ZOO_DIR' not in os.environ,
                     'HDFEOS_ZOO_DIR environment variable not set.')
    def test_first_block_subset(self):
        file = 'MISR_AM1_GRP_ELLIPSOID_GM_P117_O058421_BA_F03_0024.hdf'
        som_file = fixtures.test_file_path(file)
        gdf = GridFile(som_file)

        lat, lon = gdf.grids['BlueBand'][0, :, :]
        self.assertEqual(lat.shape, (128, 512))
        self.assertEqual(lon.shape, (128, 512))
        # First point of last block.
        np.testing.assert_almost_equal(lat[0, 0], 66.226321, 5)
        np.testing.assert_almost_equal(lon[0, 0], -68.775228, 5)

        # 2nd point of block 0
        np.testing.assert_almost_equal(lat[0, 1], 66.236, 3)
        np.testing.assert_almost_equal(lon[0, 1], -68.781, 3)

        # last point of line 1
        np.testing.assert_almost_equal(lat[0, -1], 71.093, 3)
        np.testing.assert_almost_equal(lon[0, -1], -72.336, 3)

        # last point of first block
        np.testing.assert_almost_equal(lat[-1, -1], 70.702, 3)
        np.testing.assert_almost_equal(lon[-1, -1], -75.966, 3)

    @unittest.skipIf('HDFEOS_ZOO_DIR' not in os.environ,
                     'HDFEOS_ZOO_DIR environment variable not set.')
    def test_last_block_subset(self):
        file = 'MISR_AM1_GRP_ELLIPSOID_GM_P117_O058421_BA_F03_0024.hdf'
        som_file = fixtures.test_file_path(file)
        gdf = GridFile(som_file)

        lat, lon = gdf.grids['BlueBand'][179, :, :]
        self.assertEqual(lat.shape, (128, 512))

        # First point of last block.
        np.testing.assert_almost_equal(lat[0, 0], -65.731, 3)
        np.testing.assert_almost_equal(lon[0, 0], -46.159, 3)

        # 2nd point of block 0
        np.testing.assert_almost_equal(lat[0, 1], -65.722, 3)
        np.testing.assert_almost_equal(lon[0, 1], -46.170, 3)

        # last point of line 1
        np.testing.assert_almost_equal(lat[0, -1], -61.088, 3)
        np.testing.assert_almost_equal(lon[0, -1], -50.539, 3)

        # last point of first block
        np.testing.assert_almost_equal(lat[-1, -1], -61.512, 3)
        np.testing.assert_almost_equal(lon[-1, -1], -52.990, 3)

    @unittest.skipIf('HDFEOS_ZOO_DIR' not in os.environ,
                     'HDFEOS_ZOO_DIR environment variable not set.')
    def test_som(self):
        R2D = 57.2957795131
        nline = 128
        nsample = 512
        file = 'MISR_AM1_GRP_ELLIPSOID_GM_P117_O058421_BA_F03_0024.hdf'
        file = fixtures.test_file_path(file)
        gdfid = he4.gdopen(file)
        gridid = he4.gdattach(gdfid, 'BlueBand')
        actual = he4.gdblksomoffset(gridid)
        xdim, ydim, upleft, lowright = he4.gdgridinfo(gridid)
        projcode, zonecode, spherecode, projparms = he4.gdprojinfo(gridid)
        offset = he4.gdblksomoffset(gridid)
        he4.gddetach(gridid)
        he4.gdclose(gdfid)
        misr.init(xdim, ydim, offset, upleft, lowright)
        som.inv_init(projcode, projparms, spherecode)
        nblocks = len(offset) + 1
        nblocks = 2
        lat = np.zeros((nline,nsample, nblocks))
        lon = np.zeros((nline,nsample, nblocks))

        #print_ij(0, 0,   0)
        #print_ij(0, 0,   1)
        #print_ij(0, 0, 126)
        #print_ij(0, 0, 127)
        #print_ij(0, 1,   0)
        #print_ij(0, 1,   1)
        #print_ij(0, 1, 126)
        #print_ij(0, 1, 127)
        #print_ij(0, 511, 127)
        #print_ij(179, 0,   0)
        #print_ij(179, 0,   1)
        #print_ij(179, 0,   127)
        #print_ij(179, 511, 126)
        #print_ij(179, 511, 127)


