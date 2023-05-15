#!/usr/bin/env python
import sys
import os
from pyraf import iraf

if len(sys.argv) != 6:
  print(" [usage] python3 gaoes_comp.py inid indirec outimg ref_ap ref_com")
  print("    inlist  :  Input flat image list")
  print("    indirec :  directory of RAW data")
  print("    outimg  :  output ThAr image")
  print("    ref_ap  :  Aperture reference image")
  print("    ref_com :  Wavelength reference (1D comparison) image")
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

inid    = sys.argv[1]
indirec = sys.argv[2]
outimg  = sys.argv[3]
ref_ap  = sys.argv[4]
ref_com = sys.argv[5]

iraf.set(stdimage="imt4096")
iraf.gaoes_comp(inid=inid, indirec=indirec, outimg=outimg, ref_ap=ref_ap, ref_com=ref_com)
