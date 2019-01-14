#!/usr/bin/env python

import os, sy
# import numpy as np
s
try:
  import flow, test_data
except:
  print("failed to import clay test data and H2O flow modules")
  sys.exit(1)
 
if __name__ == '__main__':
  tiles = [['.', '.', '+', '.', '.'], ['.', '#', '#', '.', '.'], ['.', '.', '.', '.', '.']]
  dim = 14
  x0 = clay_init(tiles, dim)
  y0 = 0
  tot_tiles = flow_direction(tiles, x0, y0, dim)

