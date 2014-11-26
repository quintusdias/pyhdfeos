import numpy as np

abs_offset = None
sx = None
sy = None
xc = None
yc = None

def init(nline, nsample, offset, ulc_coord, lrc_coord):

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

def inv(block, line, sample):
    n = int((block - 1) * nl * sx)
    x = (xc + n + (line * sx))
    y = yc + ((sample + abs_offset[block-1] * sy))
    return x, y

