#!/usr/bin/env python

#sys.path.append('/path/to//module/packages')

import os, sys
# import numpy as np

try:
  from test_data import clay_init, clay_readfile, clay_writefile, clay_print
  from overflow import flow, overflow
except:
  print("failed to import clay test data and H2O flow modules")
  sys.exit(1)

if(__name__ == '__main__'):
  atiles =  [['.', '.', '.', '.', '.'], ['.', '#', '#', '.', '.'], ['.', '+', '.', '.', '.']]
  btiles =  [['.', '.', '+', '.', '.'], ['.', '#', '#', '.', '.'], ['.', '.', '.', '.', '.']]
  abtiles = [['.', '.', '+', '.', '.'], ['.', '#', '#', '.', '.'], ['.', '+', '.', '.', '.']]
  tiles = [] # [[]]

  dimsrc = clay_init(tiles) # returns pair [dim, xsrc]
  dim = dimsrc[0]
  x0 = xsrc = dimsrc[1]
  y0 = 0
  cnt = tot_tiles = flow(tiles, xsrc, y0, dim) # total "wet" tiles ?
  cnts = overflow(tiles, xsrc, dim) # returns pair

