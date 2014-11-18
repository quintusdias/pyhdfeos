import os
import pkg_resources as pkg
import sys
import tempfile
import unittest

if sys.hexversion < 0x03000000:
    from StringIO import StringIO
    from mock import patch
else:
    from io import StringIO
    from unittest.mock import patch

import numpy as np

import pyhdfeos
from pyhdfeos import GridFile

from . import fixtures
from .fixtures import test_file_exists, test_file_path

somfile = 'MISR_AM1_GRP_ELLIPSOID_GM_P117_O058421_BA_F03_0024.hdf'

@unittest.skipIf(sys.hexversion < 0x03000000, "do not bother with 2.7")
class TestPrinting(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid.h5'))
        cls.test_driver_file = file

        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid219.hdf'))
        cls.test_driver_gridfile4 = file

    def setUp(self):
        pass

    def tearDown(self):
        pass

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

    def test_he4_testdriver_file(self):
        with GridFile(self.test_driver_gridfile4) as gdf:
            with patch('sys.stdout', new=StringIO()) as fake_out:
                print(gdf)
                actual = fake_out.getvalue().strip()

        expected = fixtures.geo_polar_utm_4
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

