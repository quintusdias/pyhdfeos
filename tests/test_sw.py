import os
import pkg_resources as pkg
import unittest

import numpy as np

from pyhdfeos import SwathFile
from pyhdfeos.core import DimensionMap

from .fixtures import test_file_exists, test_file_path

issue50file = 'MOD021KM.A2000055.0000.005.2010041143816.hdf'
issue53file = 'MYD03.A2002226.0000.005.2009193071127.hdf'


class TestSuite(unittest.TestCase):
    """
    """
    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Swath219.hdf'))
        cls.swathfile = file

    @unittest.skipIf(not test_file_exists(issue50file),
                     'test file not available')
    def test_issue_50(self):
        filename = test_file_path(issue50file)
        SwathFile(filename)

    @unittest.skipIf(not test_file_exists(issue53file),
                     'test file not available')
    def test_sds_attributes(self):
        """
        retrieve SDS attributes
        """
        filename = test_file_path(issue53file)
        swf = SwathFile(filename)
        field = swf.swaths['MODIS_Swath_Type_GEO'].datafields['SolarZenith']
        actual = field.attrs['units']
        expected = 'degrees'
        self.assertEqual(actual, expected)

    def test_read_geolocation_field_1d(self):
        """
        Not an actual SDS, but a vdata
        """
        swf = SwathFile(self.swathfile)
        actual = swf.swaths['Swath1'].geofields['Time'][:]
        expected = np.array([34574087.3, 34658980.5, 34743873.7,
                             34828766.9, 34913660.09999999, 34998553.3,
                             35083446.5, 35168339.7,  35253232.9,
                             35338126.09999999, 35423019.3, 35507912.5,
                             35592805.7, 35677698.9, 35762592.09999999,
                             35847485.3, 35932378.5, 36017271.7,
                             36102164.9, 36187058.09999999])
        np.testing.assert_array_almost_equal(actual, expected, decimal=1)

    def test_read_2d_32bit_floatdata(self):
        """
        """
        swf = SwathFile(self.swathfile)
        actual = swf.swaths['Swath1'].datafields['Temperature'][:]
        self.assertEqual(actual.dtype, np.float32)

    def test_read_3d_32bit_floatdata(self):
        """
        should not error out
        """
        swf = SwathFile(self.swathfile)
        actual = swf.swaths['Swath1'].datafields['Temperature_3D'][:, :, 9]
        self.assertEqual(actual.dtype, np.float32)
        self.assertEqual(actual.shape, (15, 20))


class TestMetadata4(unittest.TestCase):
    """
    """
    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__,
                                     os.path.join('data', 'Swath219.hdf'))
        cls.swathfile = file

    def test_inqswaths4(self):
        swf = SwathFile(self.swathfile)
        self.assertEqual(list(swf.swaths.keys()), ['Swath1'])

    def test_inqgeofields4(self):
        swf = SwathFile(self.swathfile)
        self.assertEqual(list(swf.swaths['Swath1'].geofields.keys()),
                         ['Time', 'Longitude', 'Latitude'])

    def test_inqdatafields4(self):
        swf = SwathFile(self.swathfile)
        self.assertEqual(list(swf.swaths['Swath1'].datafields.keys()),
                         ['Density', 'Temperature', 'Temperature_3D',
                          'Pressure', 'Spectra', 'Count'])

    def test_inqdims(self):
        swf = SwathFile(self.swathfile)
        self.assertEqual(swf.swaths['Swath1'].dims,
                         {'GeoTrack': 20,
                          'GeoXtrack': 10,
                          'Res2tr': 40,
                          'Res2xtr': 20,
                          'Bands': 15,
                          'IndxTrack': 12,
                          'Unlim': 0})

    def test_inqmaps(self):
        swf = SwathFile(self.swathfile)
        self.assertEqual(swf.swaths['Swath1'].dimmaps['GeoTrack/Res2tr'],
                         DimensionMap(offset=0, increment=2))
        self.assertEqual(swf.swaths['Swath1'].dimmaps['GeoXtrack/Res2xtr'],
                         DimensionMap(offset=1, increment=2))

    def test_datafieldinfo(self):
        swf = SwathFile(self.swathfile)
        field = swf.swaths['Swath1'].datafields['Temperature_3D']
        self.assertEqual(field.dimlist, ['Bands', 'GeoTrack', 'GeoXtrack'])
        self.assertEqual(field.dtype, np.float32)

        field = swf.swaths['Swath1'].geofields['Longitude']
        self.assertEqual(field.dimlist, ['GeoTrack', 'GeoXtrack'])
        self.assertEqual(field.dtype, np.float32)


class TestMetadata5(unittest.TestCase):
    """
    """
    @classmethod
    def setUpClass(cls):
        file = pkg.resource_filename(__name__, os.path.join('data',
                                                            'SimpleSwath.h5'))
        cls.swathfile = file

    def test_inqgeofields(self):
        swf = SwathFile(self.swathfile)
        self.assertEqual(list(swf.swaths['INDEX'].geofields.keys()),
                         ['Latitude', 'Longitude'])
        self.assertEqual(list(swf.swaths['SIMPLE'].geofields.keys()),
                         ['Time', 'Latitude', 'Longitude'])

    def test_inqdatafields(self):
        swf = SwathFile(self.swathfile)
        self.assertEqual(list(swf.swaths['INDEX'].datafields.keys()),
                         ['Indexed_Data'])
        self.assertEqual(list(swf.swaths['SIMPLE'].datafields.keys()),
                         ['Temperature', 'Count', 'Conduction'])

    def test_inqswaths(self):
        swf = SwathFile(self.swathfile)
        self.assertEqual(list(swf.swaths.keys()), ['INDEX', 'SIMPLE'])

    def test_inqdims(self):
        swf = SwathFile(self.swathfile)
        self.assertEqual(swf.swaths['INDEX'].dims,
                         {'TrackDim': 8,
                          'XtrackDim': 8,
                          'GeoDim': 6,
                          'GeoXDim': 6})
        self.assertEqual(swf.swaths['SIMPLE'].dims['DataTrack'], 10)
        self.assertEqual(swf.swaths['SIMPLE'].dims['DataXtrack'], 4)
        self.assertEqual(swf.swaths['SIMPLE'].dims['GeoTrack'], 5)
        self.assertEqual(swf.swaths['SIMPLE'].dims['GeoXtrack'], 4)
        self.assertTrue('Unlim' in swf.swaths['SIMPLE'].dims)

    def test_inqmaps(self):
        swf = SwathFile(self.swathfile)
        self.assertEqual(swf.swaths['SIMPLE'].dimmaps['GeoTrack/DataTrack'],
                         DimensionMap(offset=0, increment=2))
        self.assertEqual(swf.swaths['SIMPLE'].dimmaps['GeoXtrack/DataXtrack'],
                         DimensionMap(offset=0, increment=1))
        self.assertEqual(swf.swaths['SIMPLE'].dimmaps['GeoTrack/GeoXtrack'],
                         DimensionMap(offset=0, increment=1))
        self.assertEqual(swf.swaths['SIMPLE'].dimmaps['GeoTrack/GeoTrack'],
                         DimensionMap(offset=0, increment=1))
        self.assertEqual(len(swf.swaths['INDEX'].dimmaps), 0)
