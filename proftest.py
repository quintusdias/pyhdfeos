import numpy as np

from pyhdfeos import som, misr
from pyhdfeos.lib import he4
from pyhdfeos import GridFile

def run_old():
    R2D = 57.2957795131
    nline = 128
    nsample = 512
    file = '/opt/data/hdfeos/MISR_AM1_GRP_ELLIPSOID_GM_P117_O058421_BA_F03_0024.hdf'
    gdfid = he4.gdopen(file)
    gridid = he4.gdattach(gdfid, 'BlueBand')
    actual = he4.gdblksomoffset(gridid)
    shape, upleft, lowright = he4.gdgridinfo(gridid)
    projcode, zonecode, spherecode, projparms = he4.gdprojinfo(gridid)
    offset = he4.gdblksomoffset(gridid)
    he4.gddetach(gridid)
    he4.gdclose(gdfid)
    misr.init(shape[1], shape[0], offset, upleft, lowright)
    som.inv_init(projcode, projparms, spherecode)
    nblocks = len(offset) + 1
    lat = np.zeros((nline,nsample, nblocks))
    lon = np.zeros((nline,nsample, nblocks))
    
    for b in range(nblocks):
        print(b)
        for j in range(nline):
            for k in range(nsample):
                l = j
                s = k
                somx, somy = misr.inv(b+1, l, s)
                lon_r, lat_r = som.inv(somx, somy)
                lon[j,k,b] = lon_r * R2D
                lat[j,k,b] = lat_r * R2D
    
def run():
    R2D = 57.2957795131
    nline = 128
    nsample = 512
    file = '/opt/data/hdfeos/MISR_AM1_GRP_ELLIPSOID_GM_P117_O058421_BA_F03_0024.hdf'
    gdf = GridFile(file)
    lat, lon = gdf.grids['BlueBand'][:,:,0:5]
    
if __name__ == '__main__':
    import cProfile
    cProfile.run('run()', sort='time')
