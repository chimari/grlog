#!/usr/bin/env python
import sys
import os
from pyraf import iraf


if len(sys.argv) != 3:
  print(" [usage] python3 splot.py image line")
  print("    image   :  Imgae to plot")
  print("    line    :  Image line to plot")
  sys.exit()

  
image   = sys.argv[1]
line    = sys.argv[2]

iraf.gaoes()

iraf.set(stdimage="imt4096")
iraf.splot(images=image, line=line, band=1)

    
