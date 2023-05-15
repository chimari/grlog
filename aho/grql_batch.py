#!/usr/bin/env python
import sys
import os
from pyraf import iraf


if len(sys.argv) != 14:
  print(" [usage] python3 grql_batch.py inlist indirec ref_ap flatimg thar1d thar2d st_x ed_x ge_line ge_stx ge_edx make1d blaze mask")
  print("    inlist  :  Input file list for batch mode")
  print("    indirec :  directory of RAW data")
  print("    ref_ap  :  Aperture reference image")
  print("    flatimg :  ApNormalized flat image")
  print("    thar1d  :  1D wavelength calibrated ThAr image")
  print("    thar2d  :  2D ThAr image")
  print("    st_x    :  Start pixel number to extract (-54 in usual)")
  print("    ed_x    :  Ebd pixel number to extract (53 in usual)")
  print("    ge_line :  Spectrum line to get count")
  print("    ge_stx  :  Start pixel to get count")
  print("    ge_edx  :  End pixel to get count")
  print("    blaze   :  Blaze function")
  print("    mask    :  Mas image")
  sys.exit()

  
inlist  = sys.argv[1]
indirec = sys.argv[2]
ref_ap  = sys.argv[3]
flatimg = sys.argv[4]
thar1d  = sys.argv[5]
thar2d  = sys.argv[6]
st_x    = sys.argv[7]
ed_x    = sys.argv[8]
ge_line = sys.argv[9]
ge_stx  = sys.argv[10]
ge_edx  = sys.argv[11]
blaze   = sys.argv[12]
mask    = sys.argv[13]

iraf.gaoes()

#inid = sys.argv[1]
iraf.set(stdimage="imt4096")
iraf.grql(inid="00000000", indirec=indirec, batch="yes", inlist=inlist, interactive="no", ref_ap=ref_ap, flatimg=flatimg, thar1d=thar1d, thar2d=thar2d, st_x=st_x, ed_x=ed_x, cosmicr="yes", scatter="yes", ecfw="yes", getcnt="yes", splot="no", cr_proc="wacosm", cr_wbas=2000., sc_inte="no", ge_stx=ge_stx, ge_edx=ge_edx, ge_low=2.0, ge_high=0.0, mk1d="yes", m1_blaze=blaze, m1_mask=mask, m1_stx=60, m1_edx=4120,clean="yes")

