#!/usr/bin/env python
import sys
import os
from pyraf import iraf


if len(sys.argv) != 15:
  print(" [usage] python3 grql.py inid indirec ref_ap flatimg thar1d thar2d st_x ed_x ge_line ge_stx ge_edx make1d blaze mask sp_line")
  print("    inid    :  8 digit frame number")
  print("    indirec :  directory of RAW data")
  print("    ref_ap  :  Aperture reference image")
  print("    flatimg :  ApNormalized flat image")
  print("    thar1d  :  1D wavelength calibrated ThAr image")
  print("    thar2d  :  2D ThAr image")
  print("    st_x    :  Start pixel number to extract (default : -54)")
  print("    ed_x    :  End pixel number to extract (default : 53)")
  print("    ge_line :  Spectrum line to get count")
  print("    ge_stx  :  Start pixel to get count")
  print("    ge_edx  :  End pixel to get count")
  print("    blaze   :  Blaze function")
  print("    mask    :  Mas image")
  print("    sp_line :  Spectrum line to plot in splot (-1 : No Plot, 0 : Combined 1D spec.)")
  sys.exit()

  
inid    = sys.argv[1]
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
sp_line = sys.argv[14]

iraf.gaoes()

#inid = sys.argv[1]
iraf.set(stdimage="imt4096")

iraf.grql(inid=inid, indirec=indirec, batch="no", interactive="no", ref_ap=ref_ap, flatimg=flatimg, thar1d=thar1d, thar2d=thar2d, st_x=st_x, ed_x=ed_x, cosmicr="yes", scatter="yes", ecfw="yes", getcnt="yes", splot="no", cr_proc="wacosm", cr_wbas=2000., sc_inte="no", ge_line=ge_line, ge_stx=ge_stx, ge_edx=ge_edx, ge_low=0.5, ge_high=1.5, sp_line=sp_line, clean="yes", mk1d="yes", m1_blaze=blaze, m1_mask=mask, m1_stx=60, m1_edx=4120)
line=int(sp_line)
if line == 0:
  plotimg = 'G%socs_ecfw_1d' % inid
  iraf.splot(images=plotimg, line=1, band=1)
elif line > 0:
  plotimg = 'G%socs_ecfw' % inid
  iraf.splot(images=plotimg, line=sp_line, band=1)

  
  
