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

import pyhdfeos
from pyhdfeos import GridFile

from . import fixtures

def fullpath(fname):
    """
    Short cut for creating the full path.
    """
    return os.path.join(os.environ['HDFEOS_ZOO_DIR'], fname)

class TestPrinting(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid.h5'))
        cls.test_driver_file = file

    def setUp(self):
        self.file = fullpath("TOMS-EP_L3-TOMSEPL3_2000m0101_v8.HDF")

    def test_geo_grid(self):
        with GridFile(self.file) as gdf:
            with patch('sys.stdout', new=StringIO()) as fake_out:
                print(gdf)
                actual = fake_out.getvalue().strip()

        self.assertEqual(actual, fixtures.geographic_grid)

    def test_geo_grid_console_script(self):
        with patch('sys.stdout', new=StringIO()) as fake_out:
            with patch('sys.argv', new=['', self.file]):
                pyhdfeos.command_line.dump_metadata()
                actual = fake_out.getvalue().strip()

        self.assertEqual(actual, fixtures.geographic_grid)

    def test_geo_grid_he5(self):
        hdffile = fullpath('GSSTFYC.3.Year.1988_2008.he5')
        with GridFile(hdffile) as gdf:
            with patch('sys.stdout', new=StringIO()) as fake_out:
                print(gdf)
                actual = fake_out.getvalue().strip()

        expected = fixtures.geographic_grids_he5
        self.assertEqual(actual, expected)

    def test_ps_grid_he5(self):
        with GridFile(self.test_driver_file) as gdf:
            with patch('sys.stdout', new=StringIO()) as fake_out:
                print(gdf.grids['PolarGrid'])
                actual = fake_out.getvalue().strip()

        expected = fixtures.polar_stereographic_grid
        self.assertEqual(actual, expected)

    def test_lamaz_grid(self):
        file = fullpath("MYD29P1D.A2010133.h09v07.005.2010135182659.hdf")
        grid = 'MOD_Grid_Seaice_1km'
        with GridFile(file) as gdf:
            with patch('sys.stdout', new=StringIO()) as fake_out:
                print(gdf.grids[grid])
                actual = fake_out.getvalue().strip()

        expected = fixtures.lambert_azimuthal_grid
        self.assertEqual(actual, expected)

    def test_sinusoidal_grid(self):
        file = fullpath("MOD10A1.A2000065.h00v08.005.2008237034422.hdf")
        grid = 'MOD_Grid_Snow_500m'
        with GridFile(file) as gdf:
            with patch('sys.stdout', new=StringIO()) as fake_out:
                print(gdf.grids[grid])
                actual = fake_out.getvalue().strip()

        self.assertEqual(actual, fixtures.sinusoidal_grid)

    def test_albers_grid(self):
        file = fullpath("CONUS.annual.2012.h01v06.doy007to356.v1.5.hdf")
        grid = 'WELD_GRID'
        with GridFile(file) as gdf:
            with patch('sys.stdout', new=StringIO()) as fake_out:
                print(gdf.grids[grid])
                actual = fake_out.getvalue().strip()

        self.assertEqual(actual, fixtures.albers_grid)


