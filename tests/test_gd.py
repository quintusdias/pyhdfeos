import os
import tempfile
import unittest

import matplotlib.pyplot as plt
import numpy as np

from pyhdfeos import gd as GD
from pyhdfeos.gd import GridFile, Grid
from pyhdfeos import core

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

    @classmethod
    def tearDownClass(self):
        del self.gridfile

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
        self.assertEqual(origincode, core.HDFE_GD_UL)

    def test_pixreginfo(self):
        pixregcode = self.gridfile.grids['TOMS Level 3'].pixregcode
        self.assertEqual(pixregcode, core.HDFE_CENTER)

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
                self.assertEqual(numbertypes, [core.DFNT_FLOAT] * 4)

    def test_nentries(self):
        with GD.open(self.gridfile) as gdfid:
            with GD.attach(gdfid, 'TOMS Level 3') as gridid:
                ndims, _ = GD.nentries(gridid, core.HDFE_NENTDIM)
                self.assertEqual(ndims, 2)
                nfields, _ = GD.nentries(gridid, core.HDFE_NENTFLD)
                self.assertEqual(nfields, 4)
