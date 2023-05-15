#!/usr/bin/env python
import sys
import os
from pyraf import iraf


if len(sys.argv) != 4:
  print(" [usage] python3 gaoes_mkblaze.py inec outblz mask")
  print("    inec    :  Input Multi-order Flat spectrum")
  print("    outblz  :  Output Blaze function")
  print("    mask    :  Output Mask image")
  sys.exit()

  
inec    = sys.argv[1]
outblz  = sys.argv[2]
mask    = sys.argv[3]

iraf.gaoes()

#inid = sys.argv[1]
iraf.set(stdimage="imt4096")
iraf.gaoes_mkblaze(inec=inec, outblz=outblz, mask=mask)

    
