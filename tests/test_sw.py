import os
import pkg_resources as pkg
import tempfile
import unittest

import numpy as np

from pyhdfeos.lib import he4
from pyhdfeos import SwathFile

from . import fixtures

class TestMetadata(unittest.TestCase):
    """
    """
    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid.h5'))
        cls.test_driver_gridfile5 = file
        file = pkg.resource_filename(__name__, os.path.join('data',
                                                            'SimpleSwath.h5'))
        cls.test_driver_swathfile5 = file
        file = pkg.resource_filename(__name__, os.path.join('data', 'ZA.he5'))
        cls.test_driver_zonal_average_file = file
        file = pkg.resource_filename(__name__, os.path.join('data', 'Grid219.hdf'))
        cls.test_driver_gridfile4 = file
        file = pkg.resource_filename(__name__, os.path.join('data', 'Swath219.hdf'))
        cls.test_driver_swathfile4 = file
        file = pkg.resource_filename(__name__, os.path.join('data', 'Point219.hdf'))
        cls.test_driver_pointfile4 = file

    def test_inqswaths4(self):
        swf = SwathFile(self.test_driver_swathfile4)
        self.assertEqual(list(swf.swaths.keys()), ['Swath1'])

    def test_inqswaths5(self):
        swf = SwathFile(self.test_driver_swathfile5)
        self.assertEqual(list(swf.swaths.keys()), ['INDEX', 'SIMPLE'])


