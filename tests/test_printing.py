import os
import pkg_resources as pkg
import sys
import unittest

if sys.hexversion < 0x03000000:
    from StringIO import StringIO
    from mock import patch
else:
    from io import StringIO
    from unittest.mock import patch

import pyhdfeos
from pyhdfeos import GridFile, SwathFile, ZonalAverageFile

from . import fixtures
from .fixtures import test_file_exists, test_file_path

somfile = 'MISR_AM1_GRP_ELLIPSOID_GM_P117_O058421_BA_F03_0024.hdf'
ceafile = 'AMSR_E_L3_DailyLand_V06_20050118.hdf'
nothdfeosfile = 'SW_S3E_2003100.20053531923.hdf'


class TestPrinting(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid.h5'))
        cls.test_driver_file = file

        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Grid219.hdf'))
        cls.test_driver_gridfile4 = file

        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'ZA.he5'))
        cls.zonalavgfile = file

    def setUp(self):
        pass

    def tearDown(self):
        pass

    @unittest.skipIf(not test_file_exists(nothdfeosfile),
                     'test file not available')
    def test_not_hdfeos(self):
        file = test_file_path(nothdfeosfile)
        with patch('sys.stdout', new=StringIO()) as stdout:
            with patch('sys.argv', ['', file]):
                pyhdfeos.command_line.dump_metadata()

            actual = stdout.getvalue().strip()

        expected = fixtures.not_hdfeos
        self.assertEqual(actual, expected)

    @unittest.skipIf(not test_file_exists(ceafile), 'test file not available')
    def test_cea_grid(self):
        file = test_file_path(ceafile)
        gdf = GridFile(file)
        with patch('sys.stdout', new=StringIO()) as fake_out:
            print(gdf.grids['Ascending_Land_Grid'])
            actual = fake_out.getvalue().strip()

        expected = fixtures.cea_grid
        self.assertEqual(actual, expected)

    @unittest.skipIf(not test_file_exists(somfile), 'test file not available')
    def test_som_grid(self):
        file = test_file_path(somfile)
        gdf = GridFile(file)
        with patch('sys.stdout', new=StringIO()) as fake_out:
            print(gdf.grids['GeometricParameters'])
            actual = fake_out.getvalue().strip()

        expected = fixtures.som_grid
        self.assertEqual(actual, expected)

    def test_repr_gridfile(self):
        with GridFile(self.test_driver_file) as gdf1:
            gdf2 = eval(repr(gdf1))
            self.assertEqual(gdf1.filename, gdf2.filename)
            glist1 = [grid for grid in gdf1.grids.keys()]
            glist2 = [grid for grid in gdf2.grids.keys()]
            self.assertEqual(glist1, glist2)

    def test_zonal_average(self):
        zf = ZonalAverageFile(self.zonalavgfile)
        with patch('sys.stdout', new=StringIO()) as stdout:
            print(zf)
            actual = stdout.getvalue().strip()

        expected = fixtures.zonalavgfile
        self.assertEqual(actual, expected)

    def test_testdriver_he2_utm(self):
        with GridFile(self.test_driver_gridfile4) as gdf:
            with patch('sys.stdout', new=StringIO()) as fake_out:
                print(gdf.grids['UTMGrid'])
                actual = fake_out.getvalue().strip()

        expected = fixtures.he2_utm
        self.assertEqual(actual, expected)

    def test_testdriver_he2_polar(self):
        with GridFile(self.test_driver_gridfile4) as gdf:
            with patch('sys.stdout', new=StringIO()) as fake_out:
                print(gdf.grids['PolarGrid'])
                actual = fake_out.getvalue().strip()

        expected = fixtures.he2_polar
        self.assertEqual(actual, expected)

    def test_testdriver_he2_geo(self):
        with GridFile(self.test_driver_gridfile4) as gdf:
            with patch('sys.stdout', new=StringIO()) as fake_out:
                print(gdf.grids['GEOGrid'])
                actual = fake_out.getvalue().strip()

        # Trim off last two lines, gets rid of a floating point value that is
        # hard to match.
        actual = actual.split('\n')
        actual = '\n'.join(actual[:-2])

        expected = fixtures.he2_geo
        expected = expected.split('\n')
        expected = '\n'.join(expected[:-2])

        self.assertEqual(actual, expected)

    def test_utm_grid_he5(self):
        with GridFile(self.test_driver_file) as gdf:
            with patch('sys.stdout', new=StringIO()) as fake_out:
                print(gdf.grids['UTMGrid'])
                actual = fake_out.getvalue().strip()

        expected = fixtures.utm_grid
        self.assertEqual(actual, expected)

    def test_ps_grid_he5(self):
        with GridFile(self.test_driver_file) as gdf:
            with patch('sys.stdout', new=StringIO()) as fake_out:
                print(gdf.grids['PolarGrid'])
                actual = fake_out.getvalue().strip()
        expected = fixtures.polar_stereographic_grid
        self.assertEqual(actual, expected)


class TestSwathPrinting(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__, os.path.join('data',
                                                            'Swath219.hdf'))
        cls.swath4file = file

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_repr_swath4file(self):
        with SwathFile(self.swath4file) as swf1:
            swf2 = eval(repr(swf1))
            self.assertEqual(swf1.filename, swf2.filename)

    def test_print_swath4file(self):
        with SwathFile(self.swath4file) as swf:
            with patch('sys.stdout', new=StringIO()) as stdout:
                print(swf)
                actual = stdout.getvalue().strip()
        expected = fixtures.swath4
        self.assertEqual(actual, expected)


class TestSwath5Printing(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__, os.path.join('data',
                                                            'SimpleSwath.h5'))
        cls.swathfile = file

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_print_swathfile(self):
        self.maxDiff = None
        swf = SwathFile(self.swathfile)
        with patch('sys.stdout', new=StringIO()) as stdout:
            print(swf)
            actual = stdout.getvalue().strip()
        expected = fixtures.swath5
        self.assertEqual(actual, expected)
