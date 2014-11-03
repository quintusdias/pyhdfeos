geographic_grid = """TOMS-EP_L3-TOMSEPL3_2000m0101_v8.HDF
Grid:  TOMS Level 3
    Shape:  (180, 288)
    Upper Left (x,y):  [ -1.80000000e+08   9.00000000e+07]
    Lower Right (x,y):  [  1.80000000e+08  -9.00000000e+07]
    Projection:  Geographic
    Fields:
        Ozone:
        Reflectivity:
        Aerosol:
        Erythemal:
    Attributes:
        VerticalCoordinate:  Total Column"""

lambert_azimuthal_grid = """Grid:  MOD_Grid_Seaice_1km
    Shape:  (951, 951)
    Upper Left (x,y):  [ -476784.3255  2383921.6275]
    Lower Right (x,y):  [  476784.3255  1430352.9765]
    Projection:  Lambert Azimuthal
        Radius of reference sphere(km):  6371.228
        Center Longitude:  0.0
        Center Latitude:  90.0
        False Easting:  0.0
        False Northing:  0.0
    Fields:
        Sea_Ice_by_Reflectance:
        Sea_Ice_by_Reflectance_Spatial_QA:
        Ice_Surface_Temperature:
        Ice_Surface_Temperature_Spatial_QA:
    Attributes:"""

sinusoidal_grid = """Grid:  MOD_Grid_Snow_500m
    Shape:  (2400, 2400)
    Upper Left (x,y):  [-20015109.354      1111950.519667]
    Lower Right (x,y):  [-18903158.834333        -0.      ]
    Projection:  Sinusoidal
        Radius of reference sphere(km):  6371.007181
        Longitude of Central Meridian:  0.0
        False Easting:  0.0
        False Northing:  0.0
    Fields:
        Snow_Cover_Daily_Tile:
        Snow_Spatial_QA:
        Snow_Albedo_Daily_Tile:
        Fractional_Snow_Cover:
    Attributes:"""

albers_grid = """Grid:  WELD_GRID
    Shape:  (5000, 5000)
    Upper Left (x,y):  [-2415600.  2414800.]
    Lower Right (x,y):  [-2265600.  2264800.]
    Projection:  Albers Conical Equal Area
        Semi-major axis(km):  6378.2064
        Semi-minor axis(km):  6378.2064
        Latitude of 1st Standard Parallel:  29.03
        Latitude of 2nd Standard Parallel:  45.03
        Longitude of Central Meridian:  -96.0
        Latitude of Projection Origin:  23.0
        False Easting:  0.0
        False Northing:  0.0
    Fields:
        Band1_TOA_REF:
        Band2_TOA_REF:
        Band3_TOA_REF:
        Band4_TOA_REF:
        Band5_TOA_REF:
        Band61_TOA_BT:
        Band62_TOA_BT:
        Band7_TOA_REF:
        NDVI_TOA:
        Day_Of_Year:
        Saturation_Flag:
        DT_Cloud_State:
        ACCA_State:
        Num_Of_Obs:
    Attributes:"""
