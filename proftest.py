import cProfile
import numpy as np

from pyhdfeos import som, misr
from pyhdfeos.lib import he4
from pyhdfeos import GridFile

R2D = 57.2957795131
nline = 128
nsample = 512
file = '/opt/data/hdfeos/MISR_AM1_GRP_ELLIPSOID_GM_P117_O058421_BA_F03_0024.hdf'
gdf = GridFile(file)

def run():
    lat, lon = gdf.grids['BlueBand'][:,:,0:5]

if __name__ == '__main__':
    cProfile.run('run()',sort='time')
