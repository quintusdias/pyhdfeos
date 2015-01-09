import os
import pkg_resources as pkg
import unittest

import numpy as np

from pyhdfeos import ZonalAverageFile


class TestSuite(unittest.TestCase):
    """
    """
    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'ZA.he5'))
        cls.zafile = file

    def test_read(self):
        """
        verified via H5DUMP
        """
        zf = ZonalAverageFile(self.zafile)
        actual = zf.zas['ZA1'].fields['Spectra'][14, 25, 0:2]
        expected = np.array([0, 4.58936])
        np.testing.assert_array_almost_equal(actual, expected, decimal=5)
