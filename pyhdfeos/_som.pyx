# This file was adapted from routines found in the HDF-EOS2 Dumper project on
# December 1, 2014
# 
# Copyright 2010-2013 by The HDF Group.  
# All rights reserved.                                                      
#
cdef extern from "gctp_som.h":
    void inv_init(int a,int b,double *c,int d,char *str1,char *str2,int *e,
                  int (**func)(double, double, double*, double*))
    int sominv(double y, double x, double *lon, double *lat)

cimport numpy as np
import numpy as np

cdef int _NBLOCK = 180
cdef double abs_offset[180]
cdef double relOffset[179]
cdef double sx = 0.0
cdef double sy = 0.0
cdef double xc = 0.0
cdef double yc = 0.0

cdef misr_init(int nline, int nsample, float[:] relOff,
              double[:] ulc_coord, double[:] lrc_coord):
    """
    Parameters
    ----------
    nline : int
        number of lines in a block
    nsample : int
        number of samples in a block
    relOff : memory view of numpy.ndarray
        block offsets 
    ulc_coord, lrc_coord : memory view of numpy.ndarray
        upper left corner and lower right corner coordinates in meters
    """
    global abs_offset, nl, sx, sy, xc, yc
    cdef int i

    # convert relative offsets to absolute offsets
    abs_offset[0] = 0.0
    for i in range(_NBLOCK):
        abs_offset[i] = abs_offset[i-1] + relOff[i-1]
        relOffset[i-1] = relOff[i-i]

    # set ulc and lrc SOM coordinates.
    # Note:  ulc y and lrc y are reversed in the structural metadata
    ulc = np.array([ulc_coord[0], lrc_coord[1]])
    lrc = np.array([lrc_coord[0], ulc_coord[1]])

    # Set number of blocks, lines and samples
    nb = _NBLOCK
    nl = nline
    ns = nsample

    # Compute pixel size in ulc/lrc units (meters)
    sx = (lrc[0] - ulc[0]) / nl
    sy = (lrc[1] - ulc[1]) / ns

    xc = ulc[0] + sx / 2
    yc = ulc[1] + sy / 2

cdef misr_inv(int block, int line, int sample, double *x, double *y):
    """
    Parameters
    ----------
    block, line, sample : int
        specifies SOM block, line number, and sample number, i.e. the 
        k, i, and j coordinates of an (i,j,k) triplet
    x, y : double scalar
        output SOM X and Y coordinates 
    """
    n = int((block - 1) * nl * sx)
    x[0] = (xc + n + (line * sx))
    y[0] = yc + ((sample + abs_offset[block-1]) * sy)

def _get_som_grid(index, shape, offsets, upleft, lowright, projcode, projparms, 
                  spherecode):
    """
    Parameters
    ----------
    index : tuple
        tuple of row, column, and band slices
    shape : tuple
        dimensions of grid, should be SomBlockSize x XDim x YDim
    offsets : numpy.ndarray
        block offsets
    upleft, lowright : 2-element numpy.ndarray
        location in meters of upper left, lower right coordinates of grid
    projcode : int
        GCTP projection code
    projparms : numpy.ndarray
        projection parameters
    spherecode : int
        GCTP spheroid code
    """
    cdef double somx, somy
    cdef double lon_r, lat_r
    bands = index[0] # SOMBlocks
    rows = index[1]  # X
    cols = index[2]  # Y

    numrows = shape[1]
    numcols = shape[2]

    rows_start = 0 if rows.start is None else rows.start
    rows_step = 1 if rows.step is None else rows.step
    rows_stop = shape[1] if rows.stop is None else rows.stop
    cols_start = 0 if cols.start is None else cols.start
    cols_step = 1 if cols.step is None else cols.step
    cols_stop = shape[2] if cols.stop is None else cols.stop
    bands_start = 0 if bands.start is None else bands.start
    bands_step = 1 if bands.step is None else bands.step
    bands_stop = (len(offsets) + 1) if bands.stop is None else bands.stop

    nline = len(range(rows_start, rows_stop, rows_step))
    nsample = len(range(cols_start, cols_stop, cols_step))
    nblocks = len(range(bands_start, bands_stop, bands_step))

    R2D = 57.2957795131
    misr_init(numrows, numcols, offsets, upleft, lowright)
    inv_init_wrapper(projcode, projparms, spherecode)
    lat = np.zeros((nblocks, nline, nsample))
    lon = np.zeros((nblocks, nline, nsample))
    
    i = 0
    for b in range(bands_start, bands_stop, bands_step):
        j = 0
        for r in range(rows_start, rows_stop, rows_step):
            k = 0
            for c in range(cols_start, cols_stop, cols_step):
                l = r
                s = c
                misr_inv(b+1, l, s, &somx, &somy)
                sominv(somx, somy, &lon_r, &lat_r)
                lon[i,j,k] = lon_r * R2D
                lat[i,j,k] = lat_r * R2D
                k += 1
            j += 1
        i += 1
    
    return lat, lon

cdef inv_init_wrapper(int projcode,
                      np.ndarray[np.double_t, ndim=1] projparms,
                      int spherecode):
    """
    Wraps GCTP library call of inv_init.
    """
    #inv_init((long)projcode, (long)zonecode, (double*)projparam,
    #        (long)spherecode, NU     LL, NULL, (int*) &iflg,  inv_trans);
    cdef int iflg = 0
    cdef int (*inv_trans[201])(double, double, double*, double*)
    inv_init(<int>projcode, -1, <double *>projparms.data,
             <int>spherecode, NULL, NULL, &iflg, inv_trans)
