# cython: profile=True
import numpy as np

abs_offset = None
cdef double sx = 0.0
cdef double sy = 0.0
cdef double xc = 0.0
cdef double yc = 0.0

def misr_init(nline, nsample, offset, ulc_coord, lrc_coord):
    """
    Parameters
    ----------
    nline : int
        number of lines in a block
    nsample : int
        number of samples in a block
    offset : numpy.ndarray
        block offsets
    ulc_coord, lrc_coord : numpy.ndarray
        upper left corner and lower right corner coordinates in meters
    """

    global abs_offset, nl, sx, sy, xc, yc

    nblock = len(offset) + 1

    # convert relative offsets to absolute offsets
    abs_offset = np.cumsum(offset)
    abs_offset = np.pad(abs_offset, (1,0),
                        mode='constant', constant_values=(0,0))
    rel_offset = offset

    # set ulc and lrc SOM coordinates.
    # Note:  ulc y and lrc y are reversed in the structural metadata
    ulc = np.array([ulc_coord[0], lrc_coord[1]])
    lrc = np.array([lrc_coord[0], ulc_coord[1]])

    # Set number of blocks, lines and samples
    nb = nblock
    nl = nline
    ns = nsample

    # Compute pixel size in ulc/lrc units (meters)
    sx = (lrc[0] - ulc[0]) / nl
    sy = (lrc[1] - ulc[1]) / ns

    xc = ulc[0] + sx / 2
    yc = ulc[1] + sy / 2

def misr_inv(int block, int line, int sample):
    n = int((block - 1) * nl * sx)
    x = (xc + n + (line * sx))
    y = yc + ((sample + abs_offset[block-1]) * sy)
    return x, y


S2R = 4.848136811095359e-6

lon_center = 0
a = 0
b = 0
a2 = 0
a4 = 0
c1 = 0
c3 = 0
q = 0
t = 0
u = 0
w = 0
xj = 0
p21 = 0
sa = 0
ca = 0
es = 0
s = 0

false_easting = 0
false_northing = 0

TWO_PI = np.pi * 2
MAXLONG = 2147483647
DBLLONG = 4.61168601e18
MAX_VAL = 4

def sphdz(isph, parms):
    major = [6378206.4, 6378249.145, 6377397.155, 6378157.5,      
             6378388.0, 6378135.0, 6377276.3452, 6378145.0, 
             6378137.0, 6377563.396, 6377304.063, 6377340.189,           
             6378137.0, 6378155.0, 6378160.0, 6378245.0,                 
             6378270.0, 6378166.0, 6378150.0,                                    
             6370997.0, 6371228.0, 6371007.181, 6378273.0]
    minor = [6356583.8, 6356514.86955, 6356078.96284, 6356772.2,  
             6356911.94613, 6356750.519915, 6356075.4133,                
             6356759.769356, 6356752.31414, 6356256.91,                  
             6356103.039, 6356034.448, 6356752.314245,                   
             6356773.3205, 6356774.719, 6356863.0188,                    
             6356794.343479, 6356784.283666, 6356768.337303,             
             6370997.0, 6371228.0, 6371007.181, 6356889.4485]
    if isph == 12:
        r_major = major[isph]
        r_minor = minor[isph]
        radius = 6370997
    else:
        raise RuntimeError('unhandled code')
    return r_major, r_minor, radius

def paksz(ang):
    if ang < 0:
        fac = -1
    else:
        fac = 1
    sec = np.abs(ang)
    tmp = 1e6
    i = np.floor(sec / tmp)
    deg = i
    sec = sec - deg * 1e6
    min = np.floor(sec / 1e3)
    sec = sec - min * 1e3
    sec = fac * (deg * 3600 + min * 60 + sec)
    return sec / 3600

def som_inv_init(projcode, parms, datum):

    global false_easting, false_northing

    r_major, r_minor, radius = sphdz(datum, parms)

    false_easting = parms[6]
    false_northing = parms[7]
    path = int(parms[3])
    satnum = int(parms[2])
    if parms[12] == 0:
        mode = 1
        alf = paksz(parms[3]) * 3600 * S2R
        lon1 = paksz(parms[4]) * 3600 * S2R
        time = parms[8]
        sat_ratio = parms[9]
    else:
        raise RuntimeError("Unhandled number of blocks")
    sominvint(r_major, r_minor, satnum, path, alf, lon1, false_easting,
              false_northing, time, mode, sat_ratio)

def sominvint(r_major, r_minor, satnum, path, alf_in, lon, false_east,
              false_north, time, flag, sat_ratio):
    
    global p21, lon_center, a, b, a2, a4, c1, c3, q, t, u, w, xj, p21, sa, ca, es, s
    global false_easting, false_northing

    false_easting = false_east
    false_northing = false_north
    a = r_major
    b = r_minor
    es = 1.0 - (r_minor / r_major) ** 2
    if flag:
        alf = alf_in
        lon_center = lon
        p21 = time / 1440.0
    else:
        raise RuntimeError("unhandled flag")

    ca = np.cos(alf)
    if np.abs(ca) < 1e-9:
        ca = 1e-9
    sa = np.sin(alf)
    e2c = es * ca * ca
    e2s = es * sa * sa
    w = (1.0 - e2c) / (1.0 - es)
    w = w * w - 1.0
    one_es = 1.0 - es
    q = e2s / one_es
    t = (e2s*(2.0-es)) / (one_es*one_es)
    u = e2c / one_es
    xj = one_es * one_es * one_es
    dlam = 0.0
    fb, fa2, fa4, fc1, fc3, dlam = som_series(dlam)
    suma2 = fa2
    suma4 = fa4
    sumb = fb
    sumc1 = fc1
    sumc3 = fc3
    for i in range(9,82,18):
        dlam = i
        fb, fa2, fa4, fc1, fc3, dlam = som_series(dlam)
        suma2 += 4.0 * fa2
        suma4 += 4.0 * fa4
        sumb += 4.0 * fb
        sumc1 += 4.0 * fc1
        sumc3 += 4.0 * fc3
    for i in range(18,73,18):
        dlam = i
        fb, fa2, fa4, fc1, fc3, dlam = som_series(dlam)
        suma2 = suma2 + 2.0 * fa2
        suma4 = suma4 + 2.0 * fa4
        sumb = sumb + 2.0 * fb
        sumc1 += 2.0 * fc1
        sumc3 += 2.0 * fc3

    dlam = 90.0
    fb, fa2, fa4, fc1, fc3, dlam = som_series(dlam)
    suma2 = suma2 + fa2
    suma4 += fa4
    sumb += fb
    sumc1 += fc1
    sumc3 += fc3
    a2 = suma2 / 30
    a4 = suma4 / 60
    b = sumb / 30
    c1 = sumc1 / 15
    c3 = sumc3 / 45


def som_series(dlam):
    dlam *= 0.0174532925
    sd = np.sin(dlam)
    sdsq = sd * sd
    s = p21 * sa * np.cos(dlam) * np.sqrt((1.0+t*sdsq)/((1.0+w*sdsq)*(1.0+q*sdsq)))
    h = np.sqrt((1.0+q*sdsq)/(1.0+w*sdsq))*(((1.0+w*sdsq)/((1.0+q*sdsq)*(1.0+q*sdsq)))-p21*ca)
    sq = np.sqrt(xj * xj + s * s)
    fb = (h * xj - s * s) / sq
    fa2 = fb * np.cos(2.0 * dlam)
    fa4 = fb * np.cos(4.0 * dlam)
    fc = s * (h + xj) / sq
    fc1 = fc * np.cos(dlam)
    fc3 = fc * np.cos(3.0 * dlam)
    return fb, fa2, fa4, fc1, fc3, dlam

def som_inv(y, x):

    global false_easting, false_northing

    # Inverse equations. Begin inverse computation with approximation for tlon. 
    # Solve for transformed long.
    temp=y
    y=x - false_easting
    x= temp - false_northing
    tlon= x/(a*b)
    conv=1.e-9
    for inumb in range(50):
       sav = tlon
       sd = np.sin(tlon)
       sdsq = sd*sd;
       s = p21 * sa * np.cos(tlon) * np.sqrt((1.0+t*sdsq)/((1.0+w*sdsq)*(1.0+q*sdsq)))
       blon = (x/a)+(y/a)*s/xj-a2*np.sin(2.0*tlon)-a4*np.sin(4.0*tlon)-(s/xj)*(c1*np.sin(tlon)+c3*np.sin(3.0*tlon))
       tlon = blon/b
       dif = tlon-sav
       if np.abs(dif) < conv:
           break;
    
    # Compute transformed lat.
    # ------------------------
    st = np.sin(tlon)
    defac = np.exp(np.sqrt(1.0+s*s/xj/xj)*(y/a-c1*st-c3*np.sin(3.0*tlon)));
    actan = np.arctan(defac);
    tlat = 2.0*(actan-(np.pi/4.0));
    
    # Compute geodetic longitude
    # --------------------------*/
    dd = st*st;
    if np.abs(np.cos(tlon))<1.e-7:
        tlon = tlon-1.e-7
    bigk = np.sin(tlat)
    bigk2 = bigk*bigk
    xlamt = np.arctan(((1.0-bigk2/(1.0-es))*np.tan(tlon)*ca-bigk*sa*np.sqrt((1.0+q*dd)*(1.0-bigk2)-bigk2*u)/np.cos(tlon))/(1.0-bigk2*(1.0+u)))
    
    # Correct inverse quadrant
    # ------------------------
    if xlamt >= 0.0:
        sl = 1.0
    if xlamt<0.0:
        sl =  -1.0;
    if np.cos(tlon) >= 0.0:
        scl = 1.0;
    if np.cos(tlon) < 0.0:
        scl =  -1.0
    xlamt = xlamt-((np.pi/2.0)*(1.0-scl)*sl)
    dlon = xlamt-p21*tlon
    
    # Compute geodetic latitude
    # -------------------------
    if np.abs(sa)<1.e-7:
        dlat = np.asin(bigk/np.sqrt((1.0-es)*(1.0-es)+es*bigk2))
    if np.abs(sa) >= 1.e-7:
        dlat = np.arctan((np.tan(tlon)*np.cos(xlamt)-ca*np.sin(xlamt))/((1.0-es)*sa))

    lon  =  adjust_lon(dlon+lon_center)
    lat  =  dlat
    return lon, lat

def adjust_lon(x):
    """
    Function to adjust a longitude angle to range from -180 to 180 radians
 
    Parameters:
    x : float
        angle in radians
    """
    count = 0

    while True:
        if np.abs(x) <= np.pi:
            break
        elif np.abs(x / np.pi) < 2:
            x = x - np.sign(x) * TWO_PI
        elif np.abs(x / TWO_PI) < MAXLONG:
            x = x - np.floor(x / TWO_PI) * TWO_PI
        elif np.abs(x / (MAXLONG * TWO_PI)) < MAXLONG:
            x = x-((np.floor(x / (MAXLONG * TWO_PI))) * (TWO_PI * MAXLONG))
        elif np.abs(x / (DBLLONG * TWO_PI)) < MAXLONG:
            x = x - np.floor(x / (DBLLONG * TWO_PI)) * (TWO_PI * DBLLONG)
        else:
            x = x - np.sign(x) * TWO_PI

        count += 1
        if count > MAX_VAL:
            break

    return x

def _get_som_grid(index, shape, offsets, upleft, lowright, projcode, projparms, 
                  spherecode):
    """
    Parameters
    ----------
    index : tuple
       tuple of row, column, and band slices
    """
    rows = index[0]
    cols = index[1]
    bands = index[2]

    rows_start = 0 if rows.start is None else rows.start
    rows_step = 1 if rows.step is None else rows.step
    rows_stop = shape[0] if rows.stop is None else rows.stop
    cols_start = 0 if cols.start is None else cols.start
    cols_step = 1 if cols.step is None else cols.step
    cols_stop = shape[1] if cols.stop is None else cols.stop
    bands_start = 0 if bands.start is None else bands.start
    bands_step = 1 if bands.step is None else bands.step
    bands_stop = (len(offsets) + 1) if bands.stop is None else bands.stop

    nline = len(range(rows_start, rows_stop, rows_step))
    nsample = len(range(cols_start, cols_stop, cols_step))
    nblocks = len(range(bands_start, bands_stop, bands_step))

    R2D = 57.2957795131
    misr_init(shape[1], shape[0], offsets, upleft, lowright)
    som_inv_init(projcode, projparms, spherecode)
    lat = np.zeros((nline,nsample, nblocks))
    lon = np.zeros((nline,nsample, nblocks))
    
    i = 0
    for b in range(bands_start, bands_stop, bands_step):
        print(b)
        j = 0
        for r in range(rows_start, rows_stop, rows_step):
            k = 0
            for c in range(cols_start, cols_stop, cols_step):
                l = r
                s = c
                somx, somy = misr_inv(b+1, l, s)
                lon_r, lat_r = som_inv(somx, somy)
                lon[j,k,i] = lon_r * R2D
                lat[j,k,i] = lat_r * R2D
                k += 1
            j += 1
        i += 1
    
    return lat, lon
