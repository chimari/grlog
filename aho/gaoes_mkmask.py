#!/usr/bin/env python
import sys
import os
from pyraf import iraf


if len(sys.argv) != 3:
  print(" [usage] python3 gaoes_mkmask.py inimage mask")
  print("    inimagte:  Input Multi-order spectrum (Flat)")
  print("    mask    :  Output Mask image")
  sys.exit()

  
inimage = sys.argv[1]
mask    = sys.argv[2]

iraf.gaoes()

#inid = sys.argv[1]
iraf.set(stdimage="imt4096")
iraf.gaoes_mkmask(inimage=inimage, mask=mask)

    
