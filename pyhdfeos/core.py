"""
Required for grids, swaths.
"""
import collections

from .lib import he4, he5, hdf

class EosFile(object):
    """
    """
    def __init__(self):
        pass

    def _hdf4_attrs(self, filename, gridname, fieldname):
        """
        Retrieve field attributes using HDF4 interface.
        """

        attrs = None

        fid = hdf.hopen(filename)
        sd_id = hdf.sdstart(filename)
        hdf.vstart(fid)

        grid_ref = hdf.vfind(fid, gridname)
        grid_vg = hdf.vattach(fid, grid_ref)

        members = hdf.vgettagrefs(grid_vg)
        for tag_i, ref_i in members:
            if tag_i == hdf.DFTAG_VG:
                # Descend into a vgroup if we find it.
                vg0 = hdf.vattach(fid, ref_i)
                name = hdf.vgetname(vg0)
                if name == 'Data Fields':
                    # We want this Vgroup
                    df_members = hdf.vgettagrefs(vg0)
                    for tag_j, ref_j in df_members:
                        if tag_j == hdf.DFTAG_NDG:
                            # SDS dataset.
                            idx = hdf.sdreftoindex(sd_id, ref_j)
                            sds_id = hdf.sdselect(sd_id, idx)
                            name, dims, dtype, nattrs = hdf.sdgetinfo(sds_id)
                            if name == fieldname:
                                alst = []
                                for k in range(nattrs):
                                    info = hdf.sdattrinfo(sds_id, k)
                                    name = info[0]
                                    value = hdf.sdreadattr(sds_id, k)
                                    alst.append((name, value))
                                attrs = collections.OrderedDict(alst)
                            hdf.sdendaccess(sds_id)
                hdf.vdetach(vg0)

        hdf.vdetach(grid_vg)

        hdf.vend(fid)
        hdf.sdend(sd_id)
        hdf.hclose(fid)

        if attrs is None:
            # No attributes.
            attrs = collections.OrderedDict()
        return attrs


