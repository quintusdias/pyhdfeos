geographic_grid = """TOMS-EP_L3-TOMSEPL3_2000m0101_v8.HDF
Grid:  TOMS Level 3
    Shape:  (180, 288)
    Dimensions:
        XDim:  288
        YDim:  180
    Upper Left (x,y):  [ -1.80000000e+08   9.00000000e+07]
    Lower Right (x,y):  [  1.80000000e+08  -9.00000000e+07]
    Projection:  Geographic
    Fields:
        Ozone[YDim, XDim]:
        Reflectivity[YDim, XDim]:
        Aerosol[YDim, XDim]:
        Erythemal[YDim, XDim]:
    Grid Attributes:
        VerticalCoordinate:  Total Column"""

geographic_grids_he5 = """GSSTFYC.3.Year.1988_2008.he5
Grid:  NCEP
    Shape:  (720, 1440)
    Dimensions:
        Xdim:  1440
        Ydim:  720
    Upper Left (x,y):  [ -1.80000000e+08  -9.00000000e+07]
    Lower Right (x,y):  [  1.80000000e+08   9.00000000e+07]
    Projection:  Geographic
    Fields:
        SST[YDim, XDim]:
            _FillValue:  -999.0 ;
            LongName:  sea surface skin temperature ;
            units:  C ;
        Psea_level[YDim, XDim]:
            _FillValue:  -999.0 ;
            LongName:  sea level pressure ;
            units:  hPa ;
        Tair_2m[YDim, XDim]:
            _FillValue:  -999.0 ;
            LongName:  2m air temperature ;
            units:  C ;
        Qsat[YDim, XDim]:
            _FillValue:  -999.0 ;
            LongName:  sea surface saturation humidity ;
            units:  g/kg ;
    Grid Attributes:
Grid:  SET1
    Shape:  (720, 1440)
    Dimensions:
        Xdim:  1440
        Ydim:  720
    Upper Left (x,y):  [ -1.80000000e+08  -9.00000000e+07]
    Lower Right (x,y):  [  1.80000000e+08   9.00000000e+07]
    Projection:  Geographic
    Fields:
        E[YDim, XDim]:
            _FillValue:  -999.0 ;
            long_name:  latent heat flux ;
            units:  W/m^^2 ;
        STu[YDim, XDim]:
            _FillValue:  -999.0 ;
            long_name:  zonal wind stress ;
            units:  N/m^^2 ;
        STv[YDim, XDim]:
            _FillValue:  -999.0 ;
            long_name:  meridional wind stress ;
            units:  N/m^^2 ;
        H[YDim, XDim]:
            _FillValue:  -999.0 ;
            long_name:  sensible heat flux ;
            units:  W/m^^2 ;
        Qair[YDim, XDim]:
            _FillValue:  -999.0 ;
            long_name:  surface air (~10-m) specific humidity ;
            units:  g/kg ;
        U[YDim, XDim]:
            _FillValue:  -999.0 ;
            long_name:  10-m wind speed ;
            units:  m/s ;
        DQ[YDim, XDim]:
            _FillValue:  -999.0 ;
            long_name:  sea-air humidity difference ;
            units:  g/kg ;
        Tot_Precip_Water[YDim, XDim]:
            _FillValue:  -999.0 ;
            long_name:  total precipitable water ;
            units:  g/cm^^2 ;
    Grid Attributes:"""

lambert_azimuthal_grid = """Grid:  MOD_Grid_Seaice_1km
    Shape:  (951, 951)
    Dimensions:
    Upper Left (x,y):  [ -476784.3255  2383921.6275]
    Lower Right (x,y):  [  476784.3255  1430352.9765]
    Projection:  Lambert Azimuthal
        Radius of reference sphere(km):  6371.228
        Center Longitude:  0.0
        Center Latitude:  90.0
        False Easting:  0.0
        False Northing:  0.0
    Fields:
        Sea_Ice_by_Reflectance[YDim, XDim]:
        Sea_Ice_by_Reflectance_Spatial_QA[YDim, XDim]:
        Ice_Surface_Temperature[YDim, XDim]:
        Ice_Surface_Temperature_Spatial_QA[YDim, XDim]:
    Grid Attributes:"""

polar_stereographic_grid = """Grid:  PolarGrid
    Shape:  (100, 100)
    Dimensions:
        Bands:  3
    Upper Left (x,y):  [ 0.  0.]
    Lower Right (x,y):  [ 0.  0.]
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

sinusoidal_grid = """Grid:  MOD_Grid_Snow_500m
    Shape:  (2400, 2400)
    Dimensions:
    Upper Left (x,y):  [-20015109.354      1111950.519667]
    Lower Right (x,y):  [-18903158.834333        -0.      ]
    Projection:  Sinusoidal
        Radius of reference sphere(km):  6371.007181
        Longitude of Central Meridian:  0.0
        False Easting:  0.0
        False Northing:  0.0
    Fields:
        Snow_Cover_Daily_Tile[YDim, XDim]:
        Snow_Spatial_QA[YDim, XDim]:
        Snow_Albedo_Daily_Tile[YDim, XDim]:
        Fractional_Snow_Cover[YDim, XDim]:
    Grid Attributes:"""

albers_grid = """Grid:  WELD_GRID
    Shape:  (5000, 5000)
    Dimensions:
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
        Band1_TOA_REF[YDim, XDim]:
        Band2_TOA_REF[YDim, XDim]:
        Band3_TOA_REF[YDim, XDim]:
        Band4_TOA_REF[YDim, XDim]:
        Band5_TOA_REF[YDim, XDim]:
        Band61_TOA_BT[YDim, XDim]:
        Band62_TOA_BT[YDim, XDim]:
        Band7_TOA_REF[YDim, XDim]:
        NDVI_TOA[YDim, XDim]:
        Day_Of_Year[YDim, XDim]:
        Saturation_Flag[YDim, XDim]:
        DT_Cloud_State[YDim, XDim]:
        ACCA_State[YDim, XDim]:
        Num_Of_Obs[YDim, XDim]:
    Grid Attributes:"""
