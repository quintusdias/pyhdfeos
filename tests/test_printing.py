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

class TestPrinting(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid.h5'))
        cls.test_driver_file = file

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
