import os
import pkg_resources as pkg
import tempfile
import unittest

import numpy as np

from pyhdfeos.lib import he4
from pyhdfeos import GridFile

def fullpath(fname):
    """
    Short cut for creating the full path.
    """
    return os.path.join(os.environ['HDFEOS_ZOO_DIR'], fname)

class TestClass(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        file = fullpath("TOMS-EP_L3-TOMSEPL3_2000m0101_v8.HDF")
        cls.gridfile = GridFile(file)
        cls.file = file

        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid.h5'))
        cls.test_driver_grid_file = file
        file = pkg.resource_filename(__name__, os.path.join('data', 'ZA.he5'))
        cls.test_driver_zonal_average_file = file
        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid219.hdf'))
        cls.test_driver_gridfile4 = file
        file = pkg.resource_filename(__name__, os.path.join('data', 'Swath219.hdf'))
        cls.test_driver_swathfile4 = file
        file = pkg.resource_filename(__name__, os.path.join('data', 'Point219.hdf'))
        cls.test_driver_pointfile4 = file

    @classmethod
    def tearDownClass(self):
        del self.gridfile

    def test_context_manager(self):
        with GridFile(self.file) as gdf:
            pass
        self.assertTrue(True)

    def test_eos5_readattr(self):
        """
        hdf-eos5 module needs implementation of gdreadattr
        """
        file = fullpath('TES-Aura_L3-CH4_r0000010410_F01_07.he5')
        gdf = GridFile(file)
        self.assertTrue(True)

    def test_sample_files(self):
        """should be able to load hdfeos files, whether grid files or not

        Issue 16
        """
        GridFile(self.test_driver_grid_file)
        GridFile(self.test_driver_gridfile4)
        GridFile(self.test_driver_swathfile4)
        GridFile(self.test_driver_pointfile4)
        self.assertTrue(True)

    def test_read_he4_2d_single_ellipsis(self):
        """
        array-style indexing case of [...]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].variables['Vegetation'][:]

        expected = np.zeros((200,120), dtype=np.float32)
        for j in range(200):
            expected[j] = j + 10
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_full(self):
        """
        array-style indexing case of [:]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].variables['Vegetation'][:]

        expected = np.zeros((200,120), dtype=np.float32)
        for j in range(200):
            expected[j] = j + 10
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_full_full(self):
        """
        array-style indexing case of [:,:]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].variables['Vegetation'][:,:]

        expected = np.zeros((200,120), dtype=np.float32)
        for j in range(200):
            expected[j] = j + 10
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_row(self):
        """
        array-style indexing case of [scalar int]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].variables['Vegetation'][1]

        expected = np.ones(120, dtype=np.float32) * 11
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_slice_slice(self):
        """
        array-style indexing case of [r1;r2, c1:c2]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].variables['Vegetation'][3:5, 4:7]

        expected = np.zeros((2,3), dtype=np.float32)
        for j in range(2):
            expected[j] = j + 13
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_int_ellipsis(self):
        """
        array-style indexing case of [scalar, ...]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].variables['Vegetation'][3, ...]

        expected = np.ones(120, dtype=np.float32) * 13
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_int_slice(self):
        """
        array-style indexing case of [scalar, :]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].variables['Vegetation'][3, :]

        expected = np.ones(120, dtype=np.float32) * 13
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_int_int(self):
        """
        array-style indexing case of [scalar, scalar]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].variables['Vegetation'][3, 4]

        expected = 13
        np.testing.assert_array_equal(actual, expected)

    def test_zonal_average_file(self):
        """should be able to open zonal average file

        Issue #15
        """
        gdf = GridFile(self.test_driver_zonal_average_file)
        self.assertTrue(True)

    def test_aura_datatype(self):
        file = fullpath('OMI-Aura_L2G-OMCLDO2G_2007m0129_v002-2007m0130t174603.he5')
        with GridFile(file) as gdf:
            pass
        self.assertTrue(True)

    def test_inqgrid(self):
        self.assertEqual(list(self.gridfile.grids.keys()), ['TOMS Level 3'])

    def test_gridinfo(self):
        shape = self.gridfile.grids['TOMS Level 3'].shape
        self.assertEqual(shape, (180, 288))

        upleft = self.gridfile.grids['TOMS Level 3'].upleft
        np.testing.assert_array_equal(upleft,
                                      np.array([-180000000.0, 90000000.0]))

        lowright = self.gridfile.grids['TOMS Level 3'].lowright
        np.testing.assert_array_equal(lowright,
                                      np.array([180000000.0, -90000000.0]))

    def test_origininfo(self):
        origincode = self.gridfile.grids['TOMS Level 3'].origincode
        self.assertEqual(origincode, he4.HDFE_GD_UL)

    def test_pixreginfo(self):
        pixregcode = self.gridfile.grids['TOMS Level 3'].pixregcode
        self.assertEqual(pixregcode, he4.HDFE_CENTER)

    def test_projinfo(self):
        self.assertEqual(self.gridfile.grids['TOMS Level 3'].projcode, 0)
        self.assertEqual(self.gridfile.grids['TOMS Level 3'].zonecode, -1)
        self.assertEqual(self.gridfile.grids['TOMS Level 3'].spherecode, 0)
        projparms = self.gridfile.grids['TOMS Level 3'].projparms,
        np.testing.assert_array_equal(projparms,
                np.zeros((1, 13), dtype=np.float64));

    def test_corners(self):
        """
        should be able to supply two slice arguments
        """
        rows = slice(0,180,179)
        cols = slice(0,288,287)
        lat, lon = self.gridfile.grids['TOMS Level 3'][rows, cols]
        np.testing.assert_array_equal(lon, 
                                      np.array([[-179.375, 179.375],
                                                [-179.375, 179.375]]))
        np.testing.assert_array_equal(lat, 
                                      np.array([[89.5, 89.5],
                                                [-89.5, -89.5]]))

    def test_ellipsis(self):
        """
        using an ellipsis should retrieve the entire grid
        """
        rows = slice(0,180,179)
        cols = slice(0,288,287)
        lat, lon = self.gridfile.grids['TOMS Level 3'][...]
        self.assertEqual(lat.shape, (180, 288))
        self.assertEqual(lon.shape, (180, 288))

        # Verify the corners
        rows = slice(0,180,179)
        cols = slice(0,288,287)
        np.testing.assert_array_equal(lon[rows, cols],
                                      np.array([[-179.375, 179.375],
                                                [-179.375, 179.375]]))
        np.testing.assert_array_equal(lat[rows, cols], 
                                      np.array([[89.5, 89.5],
                                                [-89.5, -89.5]]))

    def test_colon_slice(self):
        """
        using : should retrieve the entire grid
        """
        lat, lon = self.gridfile.grids['TOMS Level 3'][:]
        self.assertEqual(lat.shape, (180, 288))
        self.assertEqual(lon.shape, (180, 288))

        # Verify the corners
        rows = slice(0,180,179)
        cols = slice(0,288,287)
        np.testing.assert_array_equal(lon[rows, cols],
                                      np.array([[-179.375, 179.375],
                                                [-179.375, 179.375]]))
        np.testing.assert_array_equal(lat[rows, cols], 
                                      np.array([[89.5, 89.5],
                                                [-89.5, -89.5]]))

    def test_ellipsis_slice(self):
        """
        combine Ellipsis with slice 
        """
        lat, lon = self.gridfile.grids['TOMS Level 3'][:,...]
        self.assertEqual(lat.shape, (180, 288))
        self.assertEqual(lon.shape, (180, 288))

        # Verify the corners
        rows = slice(0,180,179)
        cols = slice(0,288,287)
        np.testing.assert_array_equal(lon[rows, cols],
                                      np.array([[-179.375, 179.375],
                                                [-179.375, 179.375]]))
        np.testing.assert_array_equal(lat[rows, cols], 
                                      np.array([[89.5, 89.5],
                                                [-89.5, -89.5]]))

    def test_getitem_int(self):
        """
        a single integer argument is not allowed

        Doesn't make a lot of sense
        """
        with self.assertRaises(RuntimeError):
            self.gridfile.grids['TOMS Level 3'][5]

    def test_too_many_slices(self):
        """
        maximum of two slices
        """
        with self.assertRaises(RuntimeError):
            self.gridfile.grids['TOMS Level 3'][:,:,:]

    def test_slice_out_of_bounds(self):
        """
        slice arguments should not exceed grid boundaries 
        """
        with self.assertRaises(RuntimeError):
            self.gridfile.grids['TOMS Level 3'][-1:25,8:25]
        with self.assertRaises(RuntimeError):
            self.gridfile.grids['TOMS Level 3'][8:25,-1:25]
        with self.assertRaises(RuntimeError):
            self.gridfile.grids['TOMS Level 3'][8:700,8:25]
        with self.assertRaises(RuntimeError):
            self.gridfile.grids['TOMS Level 3'][8:100,8:2500]

@unittest.skip
class TestLibrary(unittest.TestCase):

    def setUp(self):
        self.gridfile = fullpath("TOMS-EP_L3-TOMSEPL3_2000m0101_v8.HDF")

    def tearDown(self):
        pass

    def test_inqfields(self):
        with GD.open(self.gridfile) as gdfid:
            with GD.attach(gdfid, 'TOMS Level 3') as gridid:
                fields, ranks, numbertypes = GD.inqfields(gridid)
                self.assertEqual(fields,
                                 ['Ozone', 'Reflectivity', 'Aerosol',
                                  'Erythemal'])
                self.assertEqual(ranks, [2, 2, 2, 2])
                self.assertEqual(numbertypes, [he4.DFNT_FLOAT] * 4)

    def test_nentries(self):
        with GD.open(self.gridfile) as gdfid:
            with GD.attach(gdfid, 'TOMS Level 3') as gridid:
                ndims, _ = GD.nentries(gridid, he4.HDFE_NENTDIM)
                self.assertEqual(ndims, 2)
                nfields, _ = GD.nentries(gridid, he4.HDFE_NENTFLD)
                self.assertEqual(nfields, 4)
