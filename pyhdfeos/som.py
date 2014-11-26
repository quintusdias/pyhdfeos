import numpy as np

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
    i = np.floor(ang / 1e6)
    deg = i
    sec = ang - deg * 1e6
    min = np.floor(sec / 1e3)
    sec = sec - min * 1e3
    sec = deg * 3600 + min * 60 + sec
    return sec / 3600

def inv_init(projcode, parms, datum):

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