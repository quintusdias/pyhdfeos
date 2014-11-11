import os

def test_file_exists(file):
    if 'HDFEOS_ZOO_DIR' in os.environ:
        fullpath = os.path.join(os.environ['HDFEOS_ZOO_DIR'], file)
        if os.path.exists(fullpath):
            return True
        else:
            return False

def test_file_path(file):
    return os.path.join(os.environ['HDFEOS_ZOO_DIR'], file)

utm_grid = """Grid:  UTMGrid
    Shape:  (200, 120)
    Dimensions:
        Time:  10
        ExtDim:  60
        Unlim:  18446744073709551615
    Upper Left (x,y):  [  210584.50041  3322395.95445]
    Lower Right (x,y):  [  813931.10959  2214162.53278]
    Sphere:  Clarke 1866
    Projection:  UTM
        UTM zone:  40
    Fields:
        Vegetation[YDim, XDim]:
    Grid Attributes:"""

som_grid = """Grid:  GeometricParameters
    Shape:  (32, 8)
    Dimensions:
        SOMBlockDim:  180
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
        SolarAzimuth[SOMBlockDim, XDim, YDim]:
            _FillValue:  -555.0 ;
        SolarZenith[SOMBlockDim, XDim, YDim]:
            _FillValue:  -555.0 ;
    Grid Attributes:
        Block_size.resolution_x:  [17600     0     0     0]
        Block_size.resolution_y:  [17600     0     0     0]
        Block_size.size_x:  [8 0 0 0]
        Block_size.size_y:  [32  0  0  0]"""

polar_stereographic_grid = """Grid:  PolarGrid
    Shape:  (100, 100)
    Dimensions:
        Bands:  3
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
        Temperature[YDim, XDim]:
        Pressure[YDim, XDim]:
    Grid Attributes:"""
