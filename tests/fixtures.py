import os


def test_file_exists(file):
    if 'HDFEOS_DATA_ROOT' in os.environ:
        fullpath = os.path.join(os.environ['HDFEOS_DATA_ROOT'], file)
        if os.path.exists(fullpath):
            return True
        else:
            return False


def test_file_path(file):
    return os.path.join(os.environ['HDFEOS_DATA_ROOT'], file)

not_hdfeos = """No HDF-EOS structures detected."""

swath5 = """SimpleSwath.h5
Swath:  INDEX
    Dimensions:
        TrackDim:  8
        XtrackDim:  8
        GeoDim:  6
        GeoXDim:  6
    Dimension Maps:
    Index Maps:
        GeoDim/TrackDim:  index=[1 2 3 5 6 7]
        GeoXDim/XtrackDim:  index=[1 2 3 5 6 7]
    Geolocation Fields:
        float32 Latitude[GeoDim, GeoXDim]:
        float32 Longitude[GeoDim, GeoXDim]:
    Data Field Group Attributes:
    Data Fields:
        float32 Indexed_Data[TrackDim, XtrackDim]:
            _FillValue:  -555.0
    Swath Attributes:
Swath:  SIMPLE
    Dimensions:
        DataTrack:  10
        DataXtrack:  4
        GeoTrack:  5
        GeoXtrack:  4
        Unlim:  18446744073709551615
    Dimension Maps:
        GeoTrack/DataTrack:  offset=0, increment=2
        GeoXtrack/DataXtrack:  offset=0, increment=1
        GeoTrack/GeoXtrack:  offset=0, increment=1
        GeoTrack/GeoTrack:  offset=0, increment=1
    Index Maps:
    Geolocation Fields:
        float64 Time[GeoTrack]:
        float64 Latitude[GeoTrack, GeoXtrack]:
        float64 Longitude[GeoTrack, GeoXtrack]:
    Data Field Group Attributes:
        GroupAttribute:  [4 3 2 1]
    Data Fields:
        float64 Temperature[DataTrack, DataXtrack]:
            _FillValue:  -777.0
        int32 Count[DataTrack]:
            LocalAttribute:  [100 200 300 400]
        float64 Conduction[DataTrack, DataXtrack]:
    Swath Attributes:
        Drift:  [1 2 3 4]"""

swath4 = """Swath219.hdf
Swath:  Swath1
    Dimensions:
        GeoTrack:  20
        GeoXtrack:  10
        Res2tr:  40
        Res2xtr:  20
        Bands:  15
        IndxTrack:  12
        Unlim:  0
    Dimension Maps:
        GeoTrack/Res2tr:  offset=0, increment=2
        GeoXtrack/Res2xtr:  offset=1, increment=2
    Index Maps:
        IndxTrack/Res2tr:  index=[ 0  1  3  6  7  8 11 12 14 24 32 39]
    Geolocation Fields:
        float64 Time[GeoTrack]:
        float32 Longitude[GeoTrack, GeoXtrack]:
        float32 Latitude[GeoTrack, GeoXtrack]:
    Data Fields:
        float32 Density[GeoTrack]:
        float32 Temperature[GeoTrack, GeoXtrack]:
        float32 Temperature_3D[Bands, GeoTrack, GeoXtrack]:
        float64 Pressure[Res2tr, Res2xtr]:
        float64 Spectra[Bands, Res2tr, Res2xtr]:
        int16 Count[Unlim]:
    Swath Attributes:
        TestAttr:  [ 3  5  7 11  0  0  0  0  0  0  0  0  0  0  0  0]"""

cea_grid = """Grid:  Ascending_Land_Grid
    Dimensions:
        XDim:  1383
        YDim:  586
    Upper Left (x,y):  [-17334194.      7344784.825]
    Lower Right (x,y):  [ 17334193.5375  -7344785.    ]
    Sphere:  Unspecified
    Projection:  CEA
        Semi-major axis(km):  6371.228
        Semi-minor axis(km):  6371.228
        Longitude of Central Meridian:  0.0
        Latitude of true scale:  30.0
        False Easting:  0.0
        False Northing:  0.0
    Fields:
        float64 A_Time[YDim, XDim]:
            _FillValue:  9999.0
        int16 A_TB06.9V (Res 1)[YDim, XDim]:
            _FillValue:  9999
        int16 A_TB06.9H (Res 1)[YDim, XDim]:
            _FillValue:  9999
        int16 A_TB10.7V (Res 1)[YDim, XDim]:
            _FillValue:  9999
        int16 A_TB10.7H (Res 1)[YDim, XDim]:
            _FillValue:  9999
        int16 A_TB18.7V (Res 1)[YDim, XDim]:
            _FillValue:  9999
        int16 A_TB18.7H (Res 1)[YDim, XDim]:
            _FillValue:  9999
        int16 A_TB36.5V (Res 1)[YDim, XDim]:
            _FillValue:  9999
        int16 A_TB36.5H (Res 1)[YDim, XDim]:
            _FillValue:  9999
        int16 A_TB36.5V (Res 4)[YDim, XDim]:
            _FillValue:  9999
        int16 A_TB36.5H (Res 4)[YDim, XDim]:
            _FillValue:  9999
        int16 A_TB89.0V (Res 4)[YDim, XDim]:
            _FillValue:  9999
        int16 A_TB89.0H (Res 4)[YDim, XDim]:
            _FillValue:  9999
        int16 A_Soil_Moisture[YDim, XDim]:
            _FillValue:  9999
        int16 A_Veg_Water_Content[YDim, XDim]:
            _FillValue:  9999
        int16 A_Land_Surface_Temp[YDim, XDim]:
            _FillValue:  9999
        int16 A_Inversion_QC_Flag[YDim, XDim]:
            _FillValue:  9999
    Grid Attributes:"""

utm_grid = """Grid:  UTMGrid
    Dimensions:
        Time:  10
        ExtDim:  60
        Unlim:  18446744073709551615
        XDim:  120
        YDim:  200
    Upper Left (x,y):  [  210584.50041  3322395.95445]
    Lower Right (x,y):  [  813931.10959  2214162.53278]
    Sphere:  Clarke 1866
    Projection:  UTM
        UTM zone:  40
    Fields:
        float32 Vegetation[YDim, XDim]:
    Grid Attributes:"""

som_grid = """Grid:  GeometricParameters
    Dimensions:
        SOMBlockDim:  180
        XDim:  8
        YDim:  32
    Upper Left (x,y):  [ 7460750.  1090650.]
    Lower Right (x,y):  [ 7601550.   527450.]
    Sphere:  WGS 84
    Projection:  Space Oblique Mercator A
        Inclination of orbit at ascending node:  98.018014
        Longitude of ascending orbit at equator:  -51.028001
        False Easting:  0.0
        False Northing:  0.0
        Period of satellite revolution:  98.88 (min)
        Satellite ratio start/end:  0.0
        End of path flag (0 = start, 1 = end):  0.0
    Fields:
        float64 SolarAzimuth[SOMBlockDim, XDim, YDim]:
            _FillValue:  -555.0
        float64 SolarZenith[SOMBlockDim, XDim, YDim]:
            _FillValue:  -555.0
    Grid Attributes:
        Block_size.resolution_x:  [17600     0     0     0]
        Block_size.resolution_y:  [17600     0     0     0]
        Block_size.size_x:  [8 0 0 0]
        Block_size.size_y:  [32  0  0  0]"""

polar_stereographic_grid = """Grid:  PolarGrid
    Dimensions:
        Bands:  3
        XDim:  100
        YDim:  100
    Upper Left (x,y):  [ 0.  0.]
    Lower Right (x,y):  [ 0.  0.]
    Sphere:  International 1967
    Projection:  Polar Stereographic
        Semi-major axis(km):  6378.2064
        Semi-minor axis(km):  6378.2064
        Longitude below pole of map:  0.0
        Latitude of true scale:  90.0
        False Easting:  0.0
        False Northing:  0.0
    Fields:
        float32 Temperature[YDim, XDim]:
        float32 Pressure[YDim, XDim]:
    Grid Attributes:"""

he2_utm = """Grid:  UTMGrid
    Dimensions:
        Time:  10
        XDim:  120
        YDim:  200
    Upper Left (x,y):  [  210584.50041  3322395.95445]
    Lower Right (x,y):  [  813931.10959  2214162.53278]
    Sphere:  Clarke 1866
    Projection:  UTM
        UTM zone:  40
    Fields:
        float32 Pollution[Time, YDim, XDim]:
            _FillValue:  -7.0
        float32 Vegetation[YDim, XDim]:
        float32 Extern[YDim, XDim]:
    Grid Attributes:
        float32:  [ 1.  0.  0.  0.]"""

he2_polar = """Grid:  PolarGrid
    Dimensions:
        Bands:  3
        XDim:  100
        YDim:  100
    Upper Left (x,y):  [ 0.  0.]
    Lower Right (x,y):  [ 0.  0.]
    Sphere:  International 1967
    Projection:  Polar Stereographic
        Semi-major axis(km):  6378.2064
        Semi-minor axis(km):  6378.2064
        Longitude below pole of map:  0.0
        Latitude of true scale:  90.0
        False Easting:  0.0
        False Northing:  0.0
    Fields:
        float32 Temperature[YDim, XDim]:
        float32 Pressure[YDim, XDim]:
        float32 Soil Dryness[YDim, XDim]:
        float64 Spectra[Bands, YDim, XDim]:
    Grid Attributes:"""

he2_geo = r"""Grid:  GEOGrid
    Dimensions:
        XDim:  60
        YDim:  40
    Upper Left (x,y):  [        0.  30000000.]
    Lower Right (x,y):  [ 15000000.  20000000.]
    Sphere:  Clarke 1866
    Projection:  Geographic
    Fields:
        float64 GeoSpectra[YDim, XDim]:
            _FillValue:  1.78006040052e-307
    Grid Attributes:"""
