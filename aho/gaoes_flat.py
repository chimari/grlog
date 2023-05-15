#!/usr/bin/env python
import sys
import os
from pyraf import iraf

if len(sys.argv) != 8:
  print(" [usage] python3 gaoes_flat.py inlist indirec outimg ref_ap new_ap st_x ed_x")
  print("    inlist  :  Input flat image list")
  print("    indirec :  directory of RAW data")
  print("    outimg  :  output flat image")
  print("    ref_ap  :  Aperture reference image")
  print("    new_ap  :  New aperture image")
  print("    st_x    :  Start pixel number to extract (-54 in usual)")
  print("    ed_x    :  Ebd pixel number to extract (53 in usual)")
  sys.exit()
  
iraf.gaoes()
iraf.apnorm1.unlearn()
iraf.apnorm1.background = ")apnormalize.background"
iraf.apnorm1.skybox = ")apnormalize.skybox"
iraf.apnorm1.weights = ")apnormalize.weights"
iraf.apnorm1.pfit = ")apnormalize.pfit"
iraf.apnorm1.clean = ")apnormalize.clean"
iraf.apnorm1.saturation = ")apnormalize.saturation"
iraf.apnorm1.readnoise = ")apnormalize.readnoise"
iraf.apnorm1.gain = ")apnormalize.gain"
iraf.apnorm1.lsigma = ")apnormalize.lsigma"
iraf.apnorm1.usigma = ")apnormalize.usigma"

inlist  = sys.argv[1]
indirec = sys.argv[2]
outimg  = sys.argv[3]
ref_ap  = sys.argv[4]
new_ap  = sys.argv[5]
st_x    = sys.argv[6]
ed_x    = sys.argv[7]

iraf.set(stdimage="imt4096")
iraf.gaoes_flat(inlist=inlist, indirec=indirec, outimg=outimg, ref_ap=ref_ap, apflag="yes", new_ap=new_ap, st_x=st_x, ed_x=ed_x, scatter="yes", normali="yes",interactive="no", imcheck="yes", ic_x1=88, ic_x2=92, ic_y1=1950, ic_y2=2100)
