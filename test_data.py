#!/usr/bin/env python

# import numpy as np
import os, sys

from random import randrange as rand
from copy import deepcopy

#try:
#  import cdata_io as cio
#except:
#  print("failed to import data_io swigged C moduke")
#  sys.exit(1)

def clay_print(tiles, dim=14):
  for y in range(0, dim):
    print('{0: >2}'.format(y), tiles[y])

def clay_init(tiles, dim=14):
  """
  Initialize a square claypit of tiles with dimensions width == height = dim.
  Return the x coord index of the H2O source tile at y == 0. 
  """
  #clay0 = ['.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.','.', '.']
  #clay[y][x] =  '.' or '#' or '|' or  '~' ... i.e. 'sand' or 'clay' or 'h2o flow' or 'h2o saturation'

  clay0 = []
  for idx in range(0,dim):
    clay0.append('.')

  clay = tiles[:] = []
  for idx in range(0,dim):
    clay.append(deepcopy(clay0)) # np.zeros((14,14))

  xsrc = rand(dim/4, dim-3) # place the fountain source randomly, but within a few tiles of the borders
  print("clay_init> xsrc:", xsrc)

  y = 0 ; clay[y][xsrc] = '+'
  tiles.append(deepcopy(clay[y]))

  for y in range(1, dim):
    for x in range(0, dim):
      yesno = rand(0,3)
      if( yesno == 0 ):
        clay[y][x] =  '#' # '|' '~'
    tiles.append(deepcopy(clay[y]))

  # return dimension and location of source time (xsrc)
  return [dim, xsrc]

def clay_writefile(clay_tiles, filename='./testclay.txt', dim=14):
   bcnt = len(clay_tiles)
   t0 = time.time()
   with open(filename, 'w+') as outfile:
     outfile.writelines(clay_tiles)

   outfile.close()
   print(time.time() - t0)
   return bcnt

def clay_readfile(clay_tiles, filename='./testclay.txt', dim=14):
   bcnt = len(clay_tiles)
   t0 = time.time()
   with open(inputPath, 'r+') as infile:
     lines = infile.readlines()

   infile.close()
   print(time.time() - t0)
   return bcnt

if(__name__ == '__main__'):
  tiles = [['.', '.', '+', '.', '.'], ['.', '#', '#', '.', '.'], ['.', '.', '.', '.', '.']]
  dimsrc = clay_init(tiles)
  dim = dimsrc[0] ; xsrc = dimsrc[1]
  clay_print(tiles, dim)



