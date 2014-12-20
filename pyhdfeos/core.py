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

    def _hdf4_attrs(self, filename, eos_vg_name, fieldname, geolocation=False):
        """
        Retrieve field attributes using HDF4 interface.

        Parameters
        ----------
        filename : str
            HDF-EOS file
        eos_vg_name : str
            either the name of an HDF-EOS grid or swath
        fieldname : str
            field being sought, if it is a merged field, it will not be
            found
        geolocation : bool
            If true, then the field being sought is a geolocation field,
            not a data field.  This is useful only for swaths.
        """

        attrs = None

        fid = hdf.hopen(filename)
        sd_id = hdf.sdstart(filename)
        hdf.vstart(fid)

        grid_ref = hdf.vfind(fid, eos_vg_name)
        grid_vg = hdf.vattach(fid, grid_ref)

        members = hdf.vgettagrefs(grid_vg)
        for tag_i, ref_i in members:
            if tag_i == hdf.DFTAG_VG:
                # Descend into a vgroup if we find it.
                vg0 = hdf.vattach(fid, ref_i)
                name = hdf.vgetname(vg0)
                if hasattr(self, 'swfid'):
                    # Swath
                    if geolocation and name == 'Geolocation Fields':
                        attrs = self.collect_attrs_from_sds_in_vgroup(sd_id, vg0, fieldname)
                    elif not geolocation and name == 'Data Fields':
                        attrs = self.collect_attrs_from_sds_in_vgroup(sd_id, vg0, fieldname)
                elif hasattr(self, 'gdfid') and name == 'Data Fields':
                    # Grid
                    attrs = self.collect_attrs_from_sds_in_vgroup(sd_id, vg0, fieldname)
                hdf.vdetach(vg0)

        hdf.vdetach(grid_vg)

        hdf.vend(fid)
        hdf.sdend(sd_id)
        hdf.hclose(fid)

        if attrs is None:
            # No attributes.
            attrs = collections.OrderedDict()
        return attrs

    def collect_attrs_from_sds_in_vgroup(self, sd_id, vgroup, fieldname):

        attrs = None
        df_members = hdf.vgettagrefs(vgroup)
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

        return attrs
