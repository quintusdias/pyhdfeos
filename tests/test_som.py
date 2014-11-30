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

    @classmethod
    def setUpClass(cls):
        file = 'MISR_AM1_GRP_ELLIPSOID_GM_P117_O058421_BA_F03_0024.hdf'
        cls.som_file = fixtures.test_file_path(file)

    def test_first_block_subset(self):
        gdf = GridFile(self.som_file)

        lat, lon = gdf.grids['BlueBand'][:, :, 0]
        self.assertEqual(lat.shape, (512, 128))
        self.assertEqual(lon.shape, (512, 128))
        # First point of last block.
        np.testing.assert_almost_equal(lat[0, 0], 66.226321, 5)
        np.testing.assert_almost_equal(lon[0, 0], -68.775228, 5)

        # 2nd point of block 0
        np.testing.assert_almost_equal(lat[0, 1], 66.224, 3)
        np.testing.assert_almost_equal(lon[0, 1], -68.799, 3)

        # last point of line 1
        np.testing.assert_almost_equal(lat[0, -1], 65.907, 3)
        np.testing.assert_almost_equal(lon[0, -1], -71.757, 3)

        # last point of first block
        np.testing.assert_almost_equal(lat[-1, -1], 70.702, 3)
        np.testing.assert_almost_equal(lon[-1, -1], -75.966, 3)

    def test_last_block_subset(self):
        gdf = GridFile(self.som_file)

        lat, lon = gdf.grids['BlueBand'][:, :, 179]
        # First point of last block.
        np.testing.assert_almost_equal(lat[0, 0], -65.731, 3)
        np.testing.assert_almost_equal(lon[0, 0], -46.159, 3)

        # 2nd point of block 0
        np.testing.assert_almost_equal(lat[0, 1], -65.735, 3)
        np.testing.assert_almost_equal(lon[0, 1], -46.181, 3)

        # last point of line 1
        np.testing.assert_almost_equal(lat[0, -1], -66.234, 3)
        np.testing.assert_almost_equal(lon[0, -1], -48.973, 3)

        # last point of first block
        np.testing.assert_almost_equal(lat[-1, -1], -61.512, 3)
        np.testing.assert_almost_equal(lon[-1, -1], -52.990, 3)

    def test_som(self):
        R2D = 57.2957795131
        nline = 128
        nsample = 512
        file = 'MISR_AM1_GRP_ELLIPSOID_GM_P117_O058421_BA_F03_0024.hdf'
        file = fixtures.test_file_path(file)
        gdfid = he4.gdopen(file)
        gridid = he4.gdattach(gdfid, 'BlueBand')
        actual = he4.gdblksomoffset(gridid)
        shape, upleft, lowright = he4.gdgridinfo(gridid)
        projcode, zonecode, spherecode, projparms = he4.gdprojinfo(gridid)
        offset = he4.gdblksomoffset(gridid)
        he4.gddetach(gridid)
        he4.gdclose(gdfid)
        misr.init(shape[1], shape[0], offset, upleft, lowright)
        som.inv_init(projcode, projparms, spherecode)
        nblocks = len(offset) + 1
        nblocks = 2
        lat = np.zeros((nline,nsample, nblocks))
        lon = np.zeros((nline,nsample, nblocks))

        # First point of block 0
        somx, somy = misr.inv(1, 0.0, 0.0)
        lon, lat = som.inv(somx, somy)
        lon *= R2D
        lat *= R2D
        np.testing.assert_almost_equal(lat, 66.226321, 5)
        np.testing.assert_almost_equal(lon, -68.775228, 5)

        # 2nd point of block 0
        somx, somy = misr.inv(1, 0.0, 1.0)
        lon, lat = som.inv(somx, somy)
        lon *= R2D
        lat *= R2D
        np.testing.assert_almost_equal(lat, 66.224, 3)
        np.testing.assert_almost_equal(lon, -68.799, 3)

        print_ij(0, 0,   0)
        print_ij(0, 0,   1)
        print_ij(0, 0, 126)
        print_ij(0, 0, 127)
        print_ij(0, 1,   0)
        print_ij(0, 1,   1)
        print_ij(0, 1, 126)
        print_ij(0, 1, 127)
        print_ij(0, 511, 127)
        print_ij(179, 0,   0)
        print_ij(179, 0,   1)
        print_ij(179, 0,   127)
        print_ij(179, 511, 126)
        print_ij(179, 511, 127)

        # Last point of block 0
        somx, somy = misr.inv(1, nline-1, nsample-1)
        lon, lat = som.inv(somx, somy)
        lon *= R2D
        lat *= R2D
        np.testing.assert_almost_equal(lat, 65.7503, 3)
        np.testing.assert_almost_equal(lon, -81.4905, 3)

        # First point of last block
        somx, somy = misr.inv(180, 0.0, 0.0)
        lon, lat = som.inv(somx, somy)
        lon *= R2D
        lat *= R2D
        np.testing.assert_almost_equal(lat, -65.7305, 3)
        np.testing.assert_almost_equal(lon, -46.1594, 3)

        # 2nd to last point of last block
        somx, somy = misr.inv(180, nline-1, nsample-2)
        lon, lat = som.inv(somx, somy)
        lon *= R2D
        lat *= R2D
        np.testing.assert_almost_equal(lat, -66.205, 3)
        np.testing.assert_almost_equal(lon, -58.841, 3)

        #for b in range(nblocks):
        #    print(b)
        #    for j in range(nline):
        #        for k in range(nsample):
        #            l = j
        #            s = k
        #            somx, somy = misr.inv(b+1, l, s)
        #            lon_r, lat_r = som.inv(somx, somy)
        #            lon[j,k,b] = lon_r * R2D
        #            lat[j,k,b] = lat_r * R2D


