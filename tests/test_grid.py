import os
import pkg_resources as pkg
import unittest

import numpy as np

from pyhdfeos.lib import he4
from pyhdfeos import GridFile

from . import fixtures

issue52file = 'SBUV2-NOAA17_L2-SBUV2N17L2_2011m1231_v01-01-2012m0905t152911.h5'


class TestNegative5(unittest.TestCase):
    """
    Negative tests for HDF-EOS5 grids.
    """
    @unittest.skipIf(not fixtures.test_file_exists(issue52file),
                     'test file not available')
    def test_issue52(self):
        """
        """
        with self.assertRaises(RuntimeError):
            GridFile(fixtures.test_file_path(issue52file))


class TestReadGridCoords(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid.h5'))
        cls.test_driver_gridfile5 = file
        file = pkg.resource_filename(__name__, os.path.join('data', 'ZA.he5'))
        cls.test_driver_zonal_average_file = file
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Grid219.hdf'))
        cls.test_driver_gridfile4 = file
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Swath219.hdf'))
        cls.test_driver_swathfile4 = file
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Point219.hdf'))
        cls.test_driver_pointfile4 = file

    def test_he5(self):
        """
        make sure that long * is cast correctly.
        """
        g4 = GridFile(self.test_driver_gridfile4)
        lat4, lon4 = g4.grids['UTMGrid'][:]
        g5 = GridFile(self.test_driver_gridfile5)
        lat5, lon5 = g5.grids['UTMGrid'][:]
        self.assertTrue(True)
        np.testing.assert_array_equal(lon4, lon5)
        np.testing.assert_array_equal(lat4, lat5)


class TestRead(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid.h5'))
        cls.test_driver_gridfile5 = file
        file = pkg.resource_filename(__name__, os.path.join('data', 'ZA.he5'))
        cls.test_driver_zonal_average_file = file
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Grid219.hdf'))
        cls.test_driver_gridfile4 = file
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Swath219.hdf'))
        cls.test_driver_swathfile4 = file
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Point219.hdf'))
        cls.test_driver_pointfile4 = file

    @unittest.skipIf('HDFEOS_ZOO_DIR' not in os.environ,
                     'HDFEOS_ZOO_DIR environment variable not set.')
    def test_som_offset(self):
        """
        test GDblkSOMoffset for hdfeos2
        """
        file = 'MISR_AM1_GRP_ELLIPSOID_GM_P117_O058421_BA_F03_0024.hdf'
        file = fixtures.test_file_path(file)
        gdfid = he4.gdopen(file)
        gridid = he4.gdattach(gdfid, 'BlueBand')
        actual = he4.gdblksomoffset(gridid)
        he4.gddetach(gridid)
        he4.gdclose(gdfid)
        self.assertEqual(len(actual), 179)

        expected_offset = [0, 16, 0, 16, 0, 0, 0, 16, 0, 0, 0, 0, 16, 0, 0,
                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -16, 0, 0, 0,
                           -16, 0, 0, -16, 0, 0, -16, 0, -16, 0, -16, 0, -16,
                           -16, 0,
                           -16, 0, -16, -16, 0, -16, -16, -16, 0, -16, -16,
                           -16, -16, 0, -16,
                           -16, -16, -16, -16, -16, -16, -16, -16, -16, -16,
                           -16, -16, -16, -16, -16,
                           -16, -16, -16, -16, -16, -16, -16, -16, -16, -32,
                           -16, -16, -16, -16, -16,
                           -16, -16, -16, -16, -16, -32, -16, -16, -16, -16,
                           -16, -16, -16, -16, -16,
                           -16, -16, -16, -16, -16, -16, -16, -16, -16, -16,
                           -16, -16, -16, -16, 0,
                           -16, -16, -16, -16, -16, 0, -16, -16, -16, 0, -16,
                           -16, 0, -16, 0,
                           -16, -16, 0, -16, 0, -16, 0, 0, -16, 0, -16, 0, 0,
                           -16, 0,
                           0, 0, 0, -16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                           0, 0, 0, 0, 0, 0, 16, 0, 0, 16, 0, 0, 16, 0]
        expected = np.array(expected_offset)
        np.testing.assert_array_equal(actual, expected)

    def test_strides(self):
        """
        array-style indexing case of [::rs, ::cs]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][::10, ::10]

        self.assertEqual(actual.shape, (20, 12))

    def test_read_he4_2d_single_ellipsis(self):
        """
        array-style indexing case of [...]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][:]

        expected = np.zeros((200, 120), dtype=np.float32)
        for j in range(200):
            expected[j] = j + 10
        np.testing.assert_array_equal(actual, expected)

    def test_read_he5_2d_single_ellipsis(self):
        """
        array-style indexing case of [...]
        """
        with GridFile(self.test_driver_gridfile5) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][:]

        expected = np.array([[10, 0], [0, 76]], dtype=np.float32)
        np.testing.assert_array_equal(actual[0:280:279, 0:180:179], expected)

    def test_read_he4_2d_full(self):
        """
        array-style indexing case of [:]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][:]

        expected = np.zeros((200, 120), dtype=np.float32)
        for j in range(200):
            expected[j] = j + 10
        np.testing.assert_array_equal(actual, expected)

    def test_read_he5_2d_full(self):
        """
        array-style indexing case of [:]
        """
        with GridFile(self.test_driver_gridfile5) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][:]

        expected = np.array([[10, 0], [0, 76]], dtype=np.float32)
        np.testing.assert_array_equal(actual[0:280:279, 0:180:179], expected)

    def test_read_he4_2d_full_full(self):
        """
        array-style indexing case of [:,:]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][:, :]

        expected = np.zeros((200, 120), dtype=np.float32)
        for j in range(200):
            expected[j] = j + 10
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_row(self):
        """
        array-style indexing case of [scalar int]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][1]

        expected = np.ones(120, dtype=np.float32) * 11
        np.testing.assert_array_equal(actual, expected)

    def test_read_he5_2d_row(self):
        """
        array-style indexing case of [scalar int]
        """
        with GridFile(self.test_driver_gridfile5) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][1]

        expected = np.ones(180, dtype=np.float32) * 11
        expected[120:180] = 0
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_slice_slice(self):
        """
        array-style indexing case of [r1;r2, c1:c2]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][3:5, 4:7]

        expected = np.zeros((2, 3), dtype=np.float32)
        for j in range(2):
            expected[j] = j + 13
        np.testing.assert_array_equal(actual, expected)

    def test_read_he5_2d_slice_slice(self):
        """
        array-style indexing case of [r1;r2, c1:c2]
        """
        with GridFile(self.test_driver_gridfile5) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][3:5, 4:7]

        expected = np.zeros((2, 3), dtype=np.float32)
        for j in range(2):
            expected[j] = j + 13
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_int_ellipsis(self):
        """
        array-style indexing case of [scalar, ...]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][3, ...]

        expected = np.ones(120, dtype=np.float32) * 13
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_int_slice(self):
        """
        array-style indexing case of [scalar, :]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][3, :]

        expected = np.ones(120, dtype=np.float32) * 13
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_int_int(self):
        """
        array-style indexing case of [scalar, scalar]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][3, 4]

        expected = 13
        np.testing.assert_array_equal(actual, expected)


class TestClass(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid.h5'))
        cls.test_driver_grid_file = file
        file = pkg.resource_filename(__name__, os.path.join('data', 'ZA.he5'))
        cls.test_driver_zonal_average_file = file
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Grid219.hdf'))
        cls.test_driver_gridfile4 = file
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Swath219.hdf'))
        cls.test_driver_swathfile4 = file
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Point219.hdf'))
        cls.test_driver_pointfile4 = file

    def test_context_manager(self):
        with GridFile(self.test_driver_gridfile4):
            pass
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
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][:]

        expected = np.zeros((200, 120), dtype=np.float32)
        for j in range(200):
            expected[j] = j + 10
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_full(self):
        """
        array-style indexing case of [:]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][:]

        expected = np.zeros((200, 120), dtype=np.float32)
        for j in range(200):
            expected[j] = j + 10
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_full_full(self):
        """
        array-style indexing case of [:,:]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][:, :]

        expected = np.zeros((200, 120), dtype=np.float32)
        for j in range(200):
            expected[j] = j + 10
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_row(self):
        """
        array-style indexing case of [scalar int]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][1]

        expected = np.ones(120, dtype=np.float32) * 11
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_slice_slice(self):
        """
        array-style indexing case of [r1;r2, c1:c2]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][3:5, 4:7]

        expected = np.zeros((2, 3), dtype=np.float32)
        for j in range(2):
            expected[j] = j + 13
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_int_ellipsis(self):
        """
        array-style indexing case of [scalar, ...]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][3, ...]

        expected = np.ones(120, dtype=np.float32) * 13
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_int_slice(self):
        """
        array-style indexing case of [scalar, :]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][3, :]

        expected = np.ones(120, dtype=np.float32) * 13
        np.testing.assert_array_equal(actual, expected)

    def test_read_he4_2d_int_int(self):
        """
        array-style indexing case of [scalar, scalar]
        """
        with GridFile(self.test_driver_gridfile4) as gdf:
            actual = gdf.grids['UTMGrid'].fields['Vegetation'][3, 4]

        expected = 13
        np.testing.assert_array_equal(actual, expected)

    def test_zonal_average_file(self):
        """should be able to open zonal average file

        Issue #15
        """
        GridFile(self.test_driver_zonal_average_file)
        self.assertTrue(True)

    def test_explicit_del_he4(self):
        """
        should not generate a ReferenceError when del is explicitly used
        """
        gdf = GridFile(self.test_driver_gridfile4)
        utm = gdf.grids['UTMGrid']
        del utm
        del gdf

    def test_explicit_del_he5(self):
        """
        should not generate a ReferenceError when del is explicitly used
        """
        gdf = GridFile(self.test_driver_grid_file)
        utm = gdf.grids['UTMGrid']
        del utm
        del gdf

    def test_corners(self):
        """
        should be able to supply two slice arguments
        """
        gdf = GridFile(self.test_driver_grid_file)
        rows = slice(0, 200, 199)
        cols = slice(0, 120, 119)
        lat, lon = gdf.grids['UTMGrid'][rows, cols]
        self.assertEqual(lat.shape, (2, 2))
        self.assertEqual(lon.shape, (2, 2))

    def test_ellipsis(self):
        """
        using an ellipsis should retrieve the entire grid
        """
        gdf = GridFile(self.test_driver_grid_file)
        lat, lon = gdf.grids['UTMGrid'][...]
        self.assertEqual(lat.shape, (200, 120))
        self.assertEqual(lon.shape, (200, 120))

    def test_colon_slice(self):
        """
        using : should retrieve the entire grid
        """
        gdf = GridFile(self.test_driver_grid_file)
        lat, lon = gdf.grids['UTMGrid'][:]
        self.assertEqual(lat.shape, (200, 120))
        self.assertEqual(lon.shape, (200, 120))

    def test_ellipsis_slice(self):
        """
        combine Ellipsis with slice
        """
        gdf = GridFile(self.test_driver_gridfile4)
        lat, lon = gdf.grids['UTMGrid'][:, ...]
        self.assertEqual(lat.shape, (200, 120))
        self.assertEqual(lon.shape, (200, 120))

    def test_getitem_int(self):
        """
        a single integer argument is not allowed

        Doesn't make a lot of sense
        """
        gdf = GridFile(self.test_driver_grid_file)
        with self.assertRaises(RuntimeError):
            gdf.grids['UTMGrid'][5]

    def test_too_many_slices(self):
        """
        maximum of two slices
        """
        gdf = GridFile(self.test_driver_grid_file)
        with self.assertRaises(RuntimeError):
            gdf.grids['UTMGrid'][:, :, :]

    def test_slice_out_of_bounds(self):
        """
        slice arguments should not exceed grid boundaries
        """
        gdf = GridFile(self.test_driver_grid_file)
        with self.assertRaises(RuntimeError):
            gdf.grids['UTMGrid'][-1:25, 8:25]
        with self.assertRaises(RuntimeError):
            gdf.grids['UTMGrid'][8:25, -1:25]
        with self.assertRaises(RuntimeError):
            gdf.grids['UTMGrid'][8:700, 8:25]
        with self.assertRaises(RuntimeError):
            gdf.grids['UTMGrid'][8:100, 8:2500]


class TestMetadata(unittest.TestCase):
    """
    """
    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid.h5'))
        cls.test_driver_gridfile5 = file
        file = pkg.resource_filename(__name__, os.path.join('data', 'ZA.he5'))
        cls.test_driver_zonal_average_file = file
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Grid219.hdf'))
        cls.test_driver_gridfile4 = file
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Swath219.hdf'))
        cls.test_driver_swathfile4 = file
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Point219.hdf'))
        cls.test_driver_pointfile4 = file

    def test_inqgrids4(self):
        gdf = GridFile(self.test_driver_gridfile4)
        self.assertEqual(list(gdf.grids.keys()), ['UTMGrid', 'PolarGrid',
                                                  'GEOGrid'])

    def test_inqgrids5(self):
        gdf = GridFile(self.test_driver_gridfile5)
        self.assertEqual(list(gdf.grids.keys()), ['GEOGrid', 'PolarGrid',
                                                  'UTMGrid'])

    def test_inqgrid_fields_4(self):
        gdf = GridFile(self.test_driver_gridfile4)
        self.assertEqual(list(gdf.grids['UTMGrid'].fields.keys()),
                         ['Pollution', 'Vegetation', 'Extern'])

    def test_inqgrid_fields_5(self):
        gdf = GridFile(self.test_driver_gridfile5)
        self.assertEqual(list(gdf.grids['UTMGrid'].fields.keys()),
                         ['Vegetation'])

    def test_fielddims_4(self):
        gdf = GridFile(self.test_driver_gridfile4)
        field = gdf.grids['UTMGrid'].fields['Vegetation']
        self.assertEqual(list(field.dimlist), ['YDim', 'XDim'])

    def test_fielddims_5(self):
        gdf = GridFile(self.test_driver_gridfile5)
        field = gdf.grids['UTMGrid'].fields['Vegetation']
        self.assertEqual(list(field.dimlist), ['YDim', 'XDim'])

    def test_gridattrs_4(self):
        gdf = GridFile(self.test_driver_gridfile4)
        self.assertEqual(list(gdf.grids['UTMGrid'].attrs.keys()),
                         ['float32'])

    def test_fieldattrs_4(self):
        gdf = GridFile(self.test_driver_gridfile4)
        field = gdf.grids['GEOGrid'].fields['GeoSpectra']
        self.assertEqual(list(field.attrs.keys()), ['_FillValue'])

    def test_gridinfo(self):
        gdf = GridFile(self.test_driver_gridfile4)
        self.assertEqual(gdf.grids['UTMGrid'].xdimsize, 120)
        self.assertEqual(gdf.grids['UTMGrid'].ydimsize, 200)

        upleft = gdf.grids['UTMGrid'].upleft
        np.testing.assert_array_equal(upleft,
                                      np.array([210584.50041, 3322395.95445]))

        lowright = gdf.grids['UTMGrid'].lowright
        np.testing.assert_array_equal(lowright,
                                      np.array([813931.10959, 2214162.53278]))

    def test_origininfo(self):
        gdf = GridFile(self.test_driver_gridfile4)
        origincode = gdf.grids['UTMGrid'].origincode
        self.assertEqual(origincode, he4.HDFE_GD_UL)

    def test_pixreginfo(self):
        gdf = GridFile(self.test_driver_gridfile4)
        pixregcode = gdf.grids['UTMGrid'].pixregcode
        self.assertEqual(pixregcode, he4.HDFE_CENTER)

    def test_projinfo(self):
        gdf = GridFile(self.test_driver_gridfile4)
        self.assertEqual(gdf.grids['UTMGrid'].projcode, 1)
        self.assertEqual(gdf.grids['UTMGrid'].zonecode, 40)
        self.assertEqual(gdf.grids['UTMGrid'].spherecode, 0)
        projparms = gdf.grids['UTMGrid'].projparms,
        np.testing.assert_array_equal(projparms,
                                      np.zeros((1, 13), dtype=np.float64))
